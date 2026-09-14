#include "NPC/Components/CPNPCVisualComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "NPC/CPBaseNPC.h"
#include "Components/WidgetComponent.h"

UCPNPCVisualComponent::UCPNPCVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPNPCVisualComponent::InitializeVisual(USkeletalMeshComponent* InMeshComp, UCapsuleComponent* InCapsuleComp,
	USkeletalMesh* DefaultMesh, UAnimSequence* DefaultAnim, const FVector& BaseScale, float CapsuleRadiusRatio)
{
	MeshComponent = InMeshComp;
	CapsuleComponent = InCapsuleComp;
	DefaultOriginalMesh = DefaultMesh;
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

		if (DefaultOriginalAnim)
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			MeshComponent->PlayAnimation(DefaultOriginalAnim, true);
		}

		FitCapsuleToCurrentMesh();
	}
}

void UCPNPCVisualComponent::PushMeshOverride(FGameplayTag EffectTag, USkeletalMesh* InMesh, UAnimSequence* InAnim, float InRadiusRatio, float InZOffset,
	const FVector& InMeshScale, float InOverrideCapsuleRadius, float InOverrideCapsuleHalfHeight)
{
	if (!EffectTag.IsValid() || !InMesh) { return; }

	MeshOverrideStack.RemoveAll([&](const FMeshOverrideEntry& E) { return E.EffectTag == EffectTag; });
	MeshOverrideStack.Add(FMeshOverrideEntry(EffectTag, InMesh, InAnim, InRadiusRatio, InZOffset, InMeshScale, InOverrideCapsuleRadius, InOverrideCapsuleHalfHeight));
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

	const float OldHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();

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

	// Giant 배율 적용
	const float ScaledTargetHalfHeight = (TargetHalfHeight > 0.0f) ? (TargetHalfHeight * CurrentScaleMultiplier) : -1.0f;
	const float ScaledTargetRadius = (TargetRadius > 0.0f) ? (TargetRadius * CurrentScaleMultiplier) : -1.0f;

	const float AutoHalfHeight = MeshBounds.BoxExtent.Z * ScaleZ;
	float FinalHalfHeight = (ScaledTargetHalfHeight > 0.0f) ? ScaledTargetHalfHeight : AutoHalfHeight;
	float FinalRadius = (ScaledTargetRadius > 0.0f) ? ScaledTargetRadius : (FinalHalfHeight * RadiusRatio);

	if (FinalRadius > FinalHalfHeight)
	{
		FinalHalfHeight = FinalRadius;
	}

	const float DeltaHalfHeight = FinalHalfHeight - OldHalfHeight;
	if (AActor* OwnerActor = GetOwner())
	{
		if (FMath::Abs(DeltaHalfHeight) > 0.1f)
		{
			OwnerActor->AddActorWorldOffset(FVector(0.f, 0.f, DeltaHalfHeight), false, nullptr, ETeleportType::TeleportPhysics);
		}
	}

	CapsuleComponent->SetCapsuleSize(FinalRadius, FinalHalfHeight, true);

	if (ACPBaseNPC* BaseNPC = Cast<ACPBaseNPC>(GetOwner()))
	{
		BaseNPC->UpdateOverheadWidgetsScale(CurrentScaleMultiplier);
	}

	static const FGameplayTag SlimeTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Slime"));
	const bool bIsSlime = MeshOverrideStack.ContainsByPredicate([&](const FMeshOverrideEntry& E) {
		return E.EffectTag == SlimeTag;
		});

	FVector TargetRelativeLoc = FVector(0.f, 0.f, -FinalHalfHeight + (ZOffset * CurrentScaleMultiplier));

	if (bIsSlime)
	{
		float BodyLocalCenterZ = -160.0f;
		float BodyHalfHeight = 44.0f;

		if (UPhysicsAsset* PhysAsset = CurrentMesh->GetPhysicsAsset())
		{
			if (PhysAsset->SkeletalBodySetups.Num() > 0)
			{
				if (USkeletalBodySetup* RootSetup = PhysAsset->SkeletalBodySetups[0].Get())
				{
					if (RootSetup->AggGeom.SphereElems.Num() > 0)
					{
						const FKSphereElem& S = RootSetup->AggGeom.SphereElems[0];
						BodyLocalCenterZ = -160.0f + S.Center.Z;
						BodyHalfHeight = S.Radius;
					}
					else if (RootSetup->AggGeom.SphylElems.Num() > 0)
					{
						const FKSphylElem& S = RootSetup->AggGeom.SphylElems[0];
						BodyLocalCenterZ = -160.0f + S.Center.Z;
						BodyHalfHeight = (S.Length * 0.5f) + S.Radius;
					}
				}
			}
		}

		const float BasePhysicsBottomCompZ = BodyLocalCenterZ - BodyHalfHeight;
		const float BaseHalfHeight = (TargetHalfHeight > 0.0f) ? TargetHalfHeight : (MeshBounds.BoxExtent.Z * GetCurrentBaseScale().Z);
		const float BaseRelativeZ = -BaseHalfHeight - BasePhysicsBottomCompZ;

		TargetRelativeLoc.Z = (BaseRelativeZ + ZOffset) * CurrentScaleMultiplier;
	}

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

	if (MeshOverrideStack.Num() > 0)
	{
		const FMeshOverrideEntry& Top = MeshOverrideStack.Last();
		TargetMesh = Top.Mesh;
		TargetAnim = Top.IdleAnim;
		bIsOverridden = true;
	}

	if (TargetMesh)
	{
		MeshComponent->SetSkeletalMesh(TargetMesh);
		MeshComponent->SetRelativeScale3D(GetCurrentBaseScale() * CurrentScaleMultiplier);
		MeshComponent->SetBoundsScale(3.0f);
		MeshComponent->SetRelativeRotation(bIsOverridden ? FRotator::ZeroRotator : DefaultOriginalRotation);

		if (TargetAnim)
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			MeshComponent->PlayAnimation(TargetAnim, true);
		}
		else
		{
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
