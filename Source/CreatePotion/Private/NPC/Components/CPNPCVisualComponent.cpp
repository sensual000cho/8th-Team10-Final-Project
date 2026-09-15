#include "NPC/Components/CPNPCVisualComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "NPC/CPBaseNPC.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h" 
#include "PhysicsEngine/AggregateGeom.h"

UCPNPCVisualComponent::UCPNPCVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPNPCVisualComponent::InitializeVisual(USkeletalMeshComponent* InMeshComp, UCapsuleComponent* InCapsuleComp,
	USkeletalMesh* DefaultMesh, TSubclassOf<UAnimInstance> InAnimClass, UAnimSequence* DefaultAnim,
	const FVector& BaseScale, float CapsuleRadiusRatio)
{
	MeshComponent = InMeshComp;
	CapsuleComponent = InCapsuleComp;
	DefaultOriginalMesh = DefaultMesh; 
	DefaultAnimClass = InAnimClass;
	DefaultOriginalAnim = DefaultAnim;
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
		else if (DefaultOriginalAnim)
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			MeshComponent->PlayAnimation(DefaultOriginalAnim, true);
		}

		FitCapsuleToCurrentMesh();
	}
}

void UCPNPCVisualComponent::PushMeshOverride(FGameplayTag EffectTag, USkeletalMesh* InMesh, UAnimSequence* InAnim, float InRadiusRatio, float InZOffset,
	const FVector& InMeshScale, float InOverrideCapsuleRadius, float InOverrideCapsuleHalfHeight, bool bInDisableAnimBlueprint)
{
	if (!EffectTag.IsValid() || !InMesh) { return; }

	MeshOverrideStack.RemoveAll([&](const FMeshOverrideEntry& E) { return E.EffectTag == EffectTag; });
	MeshOverrideStack.Add(FMeshOverrideEntry(EffectTag, InMesh, InAnim, InRadiusRatio, InZOffset, InMeshScale, InOverrideCapsuleRadius, InOverrideCapsuleHalfHeight, bInDisableAnimBlueprint));
	ApplyTopMesh();
}

void UCPNPCVisualComponent::PopMeshOverride(FGameplayTag EffectTag)
{
	if (!EffectTag.IsValid()) { return; }
	MeshOverrideStack.RemoveAll([&](const FMeshOverrideEntry& E) { return E.EffectTag == EffectTag; });
	ApplyTopMesh();
}

void UCPNPCVisualComponent::PushMaterialOverride(int32 SlotIndex, FGameplayTag EffectTag, UMaterialInterface* Material)
{
	if (!MeshComponent.IsValid() || !EffectTag.IsValid() || !Material) { return; }

	FMaterialOverrideStack& StackWrapper = MaterialOverrideStackMap.FindOrAdd(SlotIndex);
	StackWrapper.Entries.RemoveAll([&](const FMaterialOverrideEntry& E) { return E.EffectTag == EffectTag; });
	StackWrapper.Entries.Add(FMaterialOverrideEntry(EffectTag, Material));
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
	float ZOffset = 0.0f;
	float RadiusRatio = StoredCapsuleRadiusRatio;

	if (MeshOverrideStack.Num() > 0)
	{
		const FMeshOverrideEntry& Top = MeshOverrideStack.Last();
		TargetRadius = Top.OverrideCapsuleRadius;
		TargetHalfHeight = Top.OverrideCapsuleHalfHeight;
		ZOffset = Top.AdditionalZOffset;
		if (Top.CapsuleRadiusRatio > 0.0f)
		{
			RadiusRatio = Top.CapsuleRadiusRatio;
		}
	}

	const float ScaleZ = MeshComponent->GetRelativeScale3D().Z;
	const FBoxSphereBounds MeshBounds = CurrentMesh->GetBounds();

	const float ScaledTargetHalfHeight = (TargetHalfHeight > 0.0f) ? (TargetHalfHeight * CurrentScaleMultiplier) : -1.0f;
	const float ScaledTargetRadius = (TargetRadius > 0.0f) ? (TargetRadius * CurrentScaleMultiplier) : -1.0f;

	const float AutoHalfHeight = MeshBounds.BoxExtent.Z * ScaleZ;
	float FinalHalfHeight = (ScaledTargetHalfHeight > 0.0f) ? ScaledTargetHalfHeight : AutoHalfHeight;
	float FinalRadius = (ScaledTargetRadius > 0.0f) ? ScaledTargetRadius : (FinalHalfHeight * RadiusRatio);

	if (FinalRadius > FinalHalfHeight)
	{
		FinalHalfHeight = FinalRadius;
	}

	CapsuleComponent->SetCapsuleSize(FinalRadius, FinalHalfHeight, true);
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
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
	
	if (AActor* OwnerActor = GetOwner())
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
				MoveComp->UpdateComponentVelocity();
				MoveComp->SetMovementMode(MOVE_Walking);
				MoveComp->FindFloor(OwnerCharacter->GetActorLocation(), MoveComp->CurrentFloor, false);
				MoveComp->AdjustFloorHeight();
			}
		}
	}

	if (ACPBaseNPC* BaseNPC = Cast<ACPBaseNPC>(GetOwner()))
	{
		BaseNPC->UpdateOverheadWidgetsScale(CurrentScaleMultiplier);
	}
	const FVector TargetRelativeLoc = FVector(0.f, 0.f, -FinalHalfHeight + (ZOffset * CurrentScaleMultiplier));
	MeshComponent->SetRelativeLocation(TargetRelativeLoc, false, nullptr, ETeleportType::TeleportPhysics);
}

void UCPNPCVisualComponent::RestoreMeshTransformAfterRagdoll()
{
	if (!MeshComponent.IsValid() || !CapsuleComponent.IsValid()) { return; }

	const FVector StoppedMeshWorldLoc = MeshComponent->GetComponentLocation();

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
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
	{
		NewCapsuleLocation = Hit.ImpactPoint + FVector(0.f, 0.f, ExpectedHalfHeight);
	}
	else
	{
		NewCapsuleLocation = StoppedMeshWorldLoc + FVector(0.f, 0.f, ExpectedHalfHeight);
	}

	CapsuleComponent->SetWorldLocation(NewCapsuleLocation, false, nullptr, ETeleportType::TeleportPhysics);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->AttachToComponent(CapsuleComponent.Get(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	MeshComponent->InitAnim(true);

	ApplyTopMesh();

	MeshComponent->RefreshBoneTransforms();
	MeshComponent->UpdateComponentToWorld();
}
void UCPNPCVisualComponent::RefreshVisuals()
{
	ApplyTopMesh();
}

void UCPNPCVisualComponent::ApplyTopMesh()
{
	if (!MeshComponent.IsValid()) { return; }

	USkeletalMesh* TargetMesh = DefaultOriginalMesh;
	UAnimSequence* TargetAnim = DefaultOriginalAnim;
	bool bIsOverridden = false;
	bool bDisableAnimBP = false;

	if (MeshOverrideStack.Num() > 0)
	{
		const FMeshOverrideEntry& Top = MeshOverrideStack.Last();
		TargetMesh = Top.Mesh;
		TargetAnim = Top.IdleAnim;
		bIsOverridden = true;
		bDisableAnimBP = Top.bDisableAnimBlueprint;
	}

	if (TargetMesh)
	{
		MeshComponent->SetSkeletalMesh(TargetMesh);
		MeshComponent->SetRelativeScale3D(GetCurrentBaseScale() * CurrentScaleMultiplier);
		MeshComponent->SetBoundsScale(3.0f);
		MeshComponent->SetRelativeRotation(bIsOverridden ? FRotator::ZeroRotator : DefaultOriginalRotation);

		if (DefaultAnimClass && !bDisableAnimBP)
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			MeshComponent->SetAnimInstanceClass(DefaultAnimClass);
		}
		else if (TargetAnim && !bDisableAnimBP)
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			MeshComponent->PlayAnimation(TargetAnim, true);
		}
		else
		{
			MeshComponent->SetAnimInstanceClass(nullptr);
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			MeshComponent->Stop();
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

	FGameplayTagContainer CurrentActiveTags;
	for (const FMeshOverrideEntry& MeshEntry : MeshOverrideStack)
	{
		CurrentActiveTags.AddTag(MeshEntry.EffectTag);
	}

	FMaterialOverrideStack* StackWrapper = MaterialOverrideStackMap.Find(SlotIndex);
	if (StackWrapper)
	{
		for (const FMaterialOverrideEntry& Entry : StackWrapper->Entries)
		{
			CurrentActiveTags.AddTag(Entry.EffectTag);
		}
	}

	for (const FCompositeMaterialRule& Rule : CompositeRules)
	{
		if (Rule.ResultMaterial && CurrentActiveTags.HasAll(Rule.RequiredTags))
		{
			MeshComponent->SetMaterial(SlotIndex, Rule.ResultMaterial);
			return;
		}
	}

	if (StackWrapper && StackWrapper->Entries.Num() > 0)
	{
		MeshComponent->SetMaterial(SlotIndex, StackWrapper->Entries.Last().Material);
		return;
	}

	if (TObjectPtr<UMaterialInterface>* DefMat = DefaultOriginalMaterials.Find(SlotIndex))
	{
		MeshComponent->SetMaterial(SlotIndex, *DefMat);
	}
}
