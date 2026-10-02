#include "NPC/Components/CPNPCVisualComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "NPC/CPBaseNPC.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h" 
#include "Components/StaticMeshComponent.h"

UCPNPCVisualComponent::UCPNPCVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPNPCVisualComponent::InitializeVisual(USkeletalMeshComponent* InMeshComp, UCapsuleComponent* InCapsuleComp,
	USkeletalMesh* DefaultMesh, TSubclassOf<UAnimInstance> InAnimClass,
	const FVector& BaseScale, float CapsuleRadiusRatio)
{
	MeshComponent = InMeshComp;
	CapsuleComponent = InCapsuleComp;
	DefaultOriginalMesh = DefaultMesh;
	DefaultAnimClass = InAnimClass;
	BaseMeshScale = BaseScale;
	StoredCapsuleRadiusRatio = CapsuleRadiusRatio;

	if (MeshComponent.IsValid() && DefaultOriginalMesh)
	{
		DefaultOriginalRotation = MeshComponent->GetRelativeRotation();
		MeshComponent->SetSkeletalMesh(DefaultOriginalMesh);
		MeshComponent->SetRelativeScale3D(BaseMeshScale * CurrentScaleMultiplier);
		MeshComponent->SetBoundsScale(3.0f);

		DefaultOriginalMaterials.Empty();
		const int32 NumMats = MeshComponent->GetNumMaterials();
		for (int32 i = 0; i < NumMats; ++i)
		{
			DefaultOriginalMaterials.Add(i, MeshComponent->GetMaterial(i));
		}

		if (DefaultAnimClass)
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			MeshComponent->SetAnimInstanceClass(DefaultAnimClass);
		}

		FitCapsuleToCurrentMesh();
	}
}

void UCPNPCVisualComponent::PushMeshOverride(FGameplayTag EffectTag, USkeletalMesh* InMesh, TSubclassOf<UAnimInstance> InAnimClass, float InRadiusRatio,
	const FVector& InMeshScale, float InOverrideCapsuleRadius, float InOverrideCapsuleHalfHeight, const FRotator& InRotationOffset)
{
	if (!EffectTag.IsValid() || !InMesh) { return; }

	MeshOverrideStack.RemoveAll([&](const FMeshOverrideEntry& E) { return E.EffectTag == EffectTag; });
	MeshOverrideStack.Add(FMeshOverrideEntry(EffectTag, InMesh, InAnimClass, InRadiusRatio, InMeshScale, InOverrideCapsuleRadius, InOverrideCapsuleHalfHeight, InRotationOffset));
	ApplyTopMesh();
}

void UCPNPCVisualComponent::PopMeshOverride(FGameplayTag EffectTag)
{
	if (!EffectTag.IsValid()) { return; }
	MeshOverrideStack.RemoveAll([&](const FMeshOverrideEntry& E) { return E.EffectTag == EffectTag; });
	ApplyTopMesh();
}

void UCPNPCVisualComponent::PushMaterialOverride(int32 SlotIndex, FGameplayTag EffectTag, UMaterialInterface* Material, int32 Priority)
{
	if (!MeshComponent.IsValid() || !EffectTag.IsValid() || !Material) { return; }

	FMaterialOverrideStack& StackWrapper = MaterialOverrideStackMap.FindOrAdd(SlotIndex);
	StackWrapper.Entries.RemoveAll([&](const FMaterialOverrideEntry& E) { return E.EffectTag == EffectTag; });
	int32 InsertIdx = 0;
	for (; InsertIdx < StackWrapper.Entries.Num(); ++InsertIdx)
	{
		if (StackWrapper.Entries[InsertIdx].Priority > Priority)
		{
			break;
		}
	}
	StackWrapper.Entries.Insert(FMaterialOverrideEntry(EffectTag, Material, Priority), InsertIdx);
	ApplyTopMaterial(SlotIndex);
}

void UCPNPCVisualComponent::PopMaterialOverride(int32 SlotIndex, FGameplayTag EffectTag)
{
	if (!MeshComponent.IsValid()) { return; }
	if (FMaterialOverrideStack* StackWrapper = MaterialOverrideStackMap.Find(SlotIndex))
	{
		StackWrapper->Entries.RemoveAll([&](const FMaterialOverrideEntry& E) { return E.EffectTag == EffectTag; });
		ApplyTopMaterial(SlotIndex);
	}
}

void UCPNPCVisualComponent::RegisterCompositeRule(const FCompositeMaterialRule& Rule)
{
	CompositeRules.RemoveAll([&](const FCompositeMaterialRule& R) { return R.RequiredTags == Rule.RequiredTags; });

	int32 InsertIdx = 0;
	for (; InsertIdx < CompositeRules.Num(); ++InsertIdx)
	{
		if (Rule.Priority > CompositeRules[InsertIdx].Priority) { break; }
	}
	CompositeRules.Insert(Rule, InsertIdx);

	for (const auto& Pair : MaterialOverrideStackMap)
	{
		ApplyTopMaterial(Pair.Key);
	}
}

void UCPNPCVisualComponent::UnregisterCompositeRule(const FGameplayTagContainer& RequiredTags)
{
	CompositeRules.RemoveAll([&](const FCompositeMaterialRule& R) { return R.RequiredTags == RequiredTags; });
	for (const auto& Pair : MaterialOverrideStackMap)
	{
		ApplyTopMaterial(Pair.Key);
	}
}

void UCPNPCVisualComponent::SetScaleMultiplier(float Multiplier)
{
	CurrentScaleMultiplier = Multiplier;
	if (MeshComponent.IsValid())
	{
		MeshComponent->SetRelativeScale3D(GetCurrentBaseScale() * CurrentScaleMultiplier);
		FitCapsuleToCurrentMesh();
	}
}

FVector UCPNPCVisualComponent::GetCurrentBaseScale() const
{
	if (MeshOverrideStack.Num() > 0 && !MeshOverrideStack.Last().MeshScale.IsNearlyZero())
	{
		return MeshOverrideStack.Last().MeshScale;
	}
	return BaseMeshScale;
}

void UCPNPCVisualComponent::FitCapsuleToCurrentMesh()
{
	if (!MeshComponent.IsValid() || !CapsuleComponent.IsValid()) { return; }
	USkeletalMesh* CurrentMesh = MeshComponent->GetSkeletalMeshAsset();
	if (!CurrentMesh) { return; }

	float TargetRadius = -1.0f;
	float TargetHalfHeight = -1.0f;
	float RadiusRatio = StoredCapsuleRadiusRatio;

	if (MeshOverrideStack.Num() > 0)
	{
		const FMeshOverrideEntry& Top = MeshOverrideStack.Last();
		TargetRadius = Top.OverrideCapsuleRadius;
		TargetHalfHeight = Top.OverrideCapsuleHalfHeight;
		if (Top.CapsuleRadiusRatio > 0.0f)
		{
			RadiusRatio = Top.CapsuleRadiusRatio;
		}
	}

	const float ScaledTargetHalfHeight = (TargetHalfHeight > 0.0f) ? (TargetHalfHeight * CurrentScaleMultiplier) : -1.0f;
	const float ScaledTargetRadius = (TargetRadius > 0.0f) ? (TargetRadius * CurrentScaleMultiplier) : -1.0f;

	const float AutoHalfHeight = CurrentMesh->GetBounds().BoxExtent.Z * MeshComponent->GetRelativeScale3D().Z;
	float FinalHalfHeight = (ScaledTargetHalfHeight > 0.0f) ? ScaledTargetHalfHeight : AutoHalfHeight;
	float FinalRadius = (ScaledTargetRadius > 0.0f) ? ScaledTargetRadius : (FinalHalfHeight * RadiusRatio);

	if (FinalRadius > FinalHalfHeight)
	{
		FinalHalfHeight = FinalRadius;
	}

	CapsuleComponent->SetCapsuleSize(FinalRadius, FinalHalfHeight, true);

	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		if (UWorld* World = GetWorld())
		{
			FHitResult GroundHit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(OwnerActor);

			const FVector ActorLoc = OwnerActor->GetActorLocation();
			const FVector TraceStart = ActorLoc + FVector(0.f, 0.f, 200.f);
			const FVector TraceEnd = ActorLoc - FVector(0.f, 0.f, 500.f);

			if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
			{
				FVector NewLocation = ActorLoc;
				NewLocation.Z = GroundHit.ImpactPoint.Z + FinalHalfHeight;
				OwnerActor->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
			}
		}

		if (ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor))
		{
			if (UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement())
			{
				MoveComp->MaxStepHeight = FMath::Min(45.0f, FinalHalfHeight * 0.5f);
				MoveComp->bForceNextFloorCheck = true;
				MoveComp->UpdateComponentVelocity();
				MoveComp->SetMovementMode(MOVE_Walking);
				MoveComp->FindFloor(OwnerCharacter->GetActorLocation(), MoveComp->CurrentFloor, false);
				MoveComp->AdjustFloorHeight();
			}
		}

		if (ACPBaseNPC* BaseNPC = Cast<ACPBaseNPC>(OwnerActor))
		{
			BaseNPC->UpdateOverheadWidgetsScale(CurrentScaleMultiplier);
		}
	}

	MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -FinalHalfHeight), false, nullptr, ETeleportType::TeleportPhysics);
}

void UCPNPCVisualComponent::RestoreMeshTransformAfterRagdoll()
{
	if (!MeshComponent.IsValid() || !CapsuleComponent.IsValid()) { return; }

	const FVector StoppedMeshWorldLoc = MeshComponent->GetComponentLocation();

	MeshComponent->PutAllRigidBodiesToSleep();
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetEnableGravity(false);
	MeshComponent->SetUseCCD(false);
	MeshComponent->SetCollisionProfileName(TEXT("CharacterMesh"));
	MeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	MeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	MeshComponent->RecreatePhysicsState();

	float ExpectedHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(CapsuleComponent->GetOwner());

	const FVector TraceStart = StoppedMeshWorldLoc + FVector(0.f, 0.f, 100.f);
	const FVector TraceEnd = StoppedMeshWorldLoc - FVector(0.f, 0.f, 3000.f);

	FVector NewCapsuleLocation = StoppedMeshWorldLoc;
	if (UWorld* World = GetWorld())
	{
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
		{
			NewCapsuleLocation = Hit.ImpactPoint + FVector(0.f, 0.f, ExpectedHalfHeight);
		}
		else
		{
			NewCapsuleLocation = StoppedMeshWorldLoc + FVector(0.f, 0.f, ExpectedHalfHeight);
		}
	}

	CapsuleComponent->SetWorldLocation(NewCapsuleLocation, false, nullptr, ETeleportType::TeleportPhysics);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->AttachToComponent(CapsuleComponent.Get(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	MeshComponent->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -ExpectedHalfHeight), DefaultOriginalRotation, false, nullptr, ETeleportType::TeleportPhysics);
	MeshComponent->InitAnim(true);

	ApplyTopMesh();

	MeshComponent->RefreshBoneTransforms();
	MeshComponent->UpdateComponentToWorld();
}
void UCPNPCVisualComponent::RefreshVisuals()
{
	ApplyTopMesh();
}

void UCPNPCVisualComponent::AttachSlimeAccessory(UStaticMesh* InMesh, FName SocketName, const FTransform& RelativeTransform)
{
	if (!InMesh || !MeshComponent.IsValid()) { return; }

	if (!SlimeAccessoryComp)
	{
		SlimeAccessoryComp = NewObject<UStaticMeshComponent>(GetOwner(), TEXT("SlimeAccessoryComp"));
		SlimeAccessoryComp->RegisterComponent();
		SlimeAccessoryComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SlimeAccessoryComp->SetCastShadow(true);
	}

	SlimeAccessoryComp->SetStaticMesh(InMesh);
	SlimeAccessoryComp->AttachToComponent(MeshComponent.Get(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	SlimeAccessoryComp->SetRelativeTransform(RelativeTransform);

	DefaultAccessoryMaterials.Empty();
	const int32 NumMats = SlimeAccessoryComp->GetNumMaterials();
	for (int32 i = 0; i < NumMats; ++i)
	{
		DefaultAccessoryMaterials.Add(i, SlimeAccessoryComp->GetMaterial(i));
	}
	ApplyTopMaterial(0);
}

void UCPNPCVisualComponent::DetachSlimeAccessory()
{
	if (SlimeAccessoryComp)
	{
		SlimeAccessoryComp->DestroyComponent();
		SlimeAccessoryComp = nullptr;
		DefaultAccessoryMaterials.Empty();
	}
}

void UCPNPCVisualComponent::ApplyTopMesh()
{
	if (!MeshComponent.IsValid()) { return; }

	USkeletalMesh* TargetMesh = DefaultOriginalMesh;
	TSubclassOf<UAnimInstance> TargetAnimClass = DefaultAnimClass;
	FRotator TargetRotation = DefaultOriginalRotation;

	if (MeshOverrideStack.Num() > 0)
	{
		const FMeshOverrideEntry& Top = MeshOverrideStack.Last();
		TargetMesh = Top.Mesh;
		TargetRotation = Top.OverrideRotationOffset;

		if (Top.OverrideAnimClass)
		{
			TargetAnimClass = Top.OverrideAnimClass;
		}
	}

	if (TargetMesh)
	{
		MeshComponent->SetSkeletalMesh(TargetMesh);
		MeshComponent->SetRelativeScale3D(GetCurrentBaseScale() * CurrentScaleMultiplier);
		MeshComponent->SetBoundsScale(3.0f);
		MeshComponent->SetRelativeRotation(TargetRotation, false, nullptr, ETeleportType::TeleportPhysics);

		if (TargetAnimClass)
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			MeshComponent->SetAnimInstanceClass(TargetAnimClass);
		}
		else
		{
			MeshComponent->SetAnimInstanceClass(nullptr);
		}

		FitCapsuleToCurrentMesh();

		for (const auto& Pair : MaterialOverrideStackMap)
		{
			ApplyTopMaterial(Pair.Key);
		}
	}
}

void UCPNPCVisualComponent::ApplyTopMaterial(int32 SlotIndex)
{
	if (!MeshComponent.IsValid()) { return; }

	UMaterialInterface* MaterialToApply = nullptr;
	FMaterialOverrideStack* StackWrapper = MaterialOverrideStackMap.Find(SlotIndex);

	if (StackWrapper && StackWrapper->Entries.Num() > 0)
	{
		MaterialToApply = StackWrapper->Entries.Last().Material;
	}
	else if (TObjectPtr<UMaterialInterface>* DefMat = DefaultOriginalMaterials.Find(SlotIndex))
	{
		MaterialToApply = *DefMat;
	}

	if (MaterialToApply)
	{
		MeshComponent->SetMaterial(SlotIndex, MaterialToApply);
	}

	if (SlimeAccessoryComp)
	{
		if (StackWrapper && StackWrapper->Entries.Num() > 0)
		{
			const FMaterialOverrideEntry& TopEntry = StackWrapper->Entries.Last();
			if (TopEntry.Priority >= 10)
			{
				SlimeAccessoryComp->SetMaterial(SlotIndex, TopEntry.Material);
				return;
			}
		}
		if (TObjectPtr<UMaterialInterface>* AccDefMat = DefaultAccessoryMaterials.Find(SlotIndex))
		{
			SlimeAccessoryComp->SetMaterial(SlotIndex, *AccDefMat);
		}
	}
}