#include "Items/Potion/CPPotionImpactComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NPC/CPQuestNPC.h"
#include "NPC/CPAnimalNPC.h"

UCPPotionImpactComponent::UCPPotionImpactComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	//Impact로 어떤 오브젝트틀을 검사할지
	EffectObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	EffectObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	EffectObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	EffectObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
}

bool UCPPotionImpactComponent::EnableImpactProcessing(APawn* InInstigator)
{
	if (bImpactTriggered || !IsValid(InInstigator) || !IsValid(GetOwner()) || !GetOwner()->HasAuthority())
	{
		return false;
	}

	ImpactInstigator = InInstigator;
	bImpactProcessingEnabled = true;

	return true;
}

void UCPPotionImpactComponent::DisableImpactProcessing()
{
	bImpactProcessingEnabled = false;
	ImpactInstigator = nullptr;
}

bool UCPPotionImpactComponent::TryTriggerPotionImpact(const FHitResult& HitResult, const TArray<FGameplayTag>& PotionEffectTags)
{
	FHitResult NormalizedHitResult = HitResult;
	NormalizedHitResult.ImpactNormal = HitResult.ImpactNormal.GetSafeNormal();

	if (NormalizedHitResult.ImpactNormal.IsNearlyZero())
	{
		NormalizedHitResult.ImpactNormal = FVector::UpVector;
	}

	NormalizedHitResult.Normal = NormalizedHitResult.ImpactNormal;
	return TryCommitPotionImpact(NormalizedHitResult, PotionEffectTags);
}

bool UCPPotionImpactComponent::TryCommitPotionImpact(const FHitResult& HitResult, const TArray<FGameplayTag>& PotionEffectTags)
{
	if (!bImpactProcessingEnabled || bImpactTriggered || !IsValid(ImpactInstigator) || !IsValid(GetOwner()) || !GetOwner()->HasAuthority() || EffectRadius <= 0.0f)
	{
		return false;
	}

	// 반복 Hit와 Gameplay Event 재진입보다 먼저 첫 Impact를 확정한다.
	bImpactTriggered = true;
	bImpactProcessingEnabled = false;
	ResolvePotionEffectArea(HitResult, PotionEffectTags);
	ImpactInstigator = nullptr;

	return true;
}

void UCPPotionImpactComponent::ResolvePotionEffectArea(const FHitResult& HitResult, const TArray<FGameplayTag>& PotionEffectTags)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());

	TArray<AActor*> OverlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(this, HitResult.ImpactPoint, EffectRadius, EffectObjectTypes, AActor::StaticClass(), ActorsToIgnore, OverlappedActors);

	TSet<AActor*> ProcessedActors;
	TSet<UAbilitySystemComponent*> ProcessedAbilitySystems;
	UAbilitySystemComponent* SourceAbilitySystem = ResolveSourceAbilitySystem();
	TArray<AActor*> EffectTargets;

	for (AActor* TargetActor : OverlappedActors)
	{
		if (!IsValid(TargetActor) || ProcessedActors.Contains(TargetActor)) continue;

		ProcessedActors.Add(TargetActor);
		
		if (ACPQuestNPC* LabNPC = Cast<ACPQuestNPC>(TargetActor)){
			LabNPC->HandleThrownPotionImpact(PotionEffectTags);
		}

		if (ACPAnimalNPC* AnimalNPC = Cast<ACPAnimalNPC>(TargetActor)) {
			AnimalNPC->HandleThrownPotionImpact(PotionEffectTags);
		}
		
		UAbilitySystemComponent* TargetAbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!IsValid(SourceAbilitySystem)) continue;
		if (!IsValid(TargetAbilitySystem) || ProcessedAbilitySystems.Contains(TargetAbilitySystem)) continue;

		ProcessedAbilitySystems.Add(TargetAbilitySystem);
		EffectTargets.Add(TargetActor);
	}

	// 최초 적중 대상에만 적용하며, 배열 순서와 관계없이 Slime을 먼저 완료한다.
	const FGameplayTag SlimeTag = FGameplayTag::RequestGameplayTag(TEXT("Potion.Effect.Slime"));
	if (PotionEffectTags.Contains(SlimeTag))
	{
		for (AActor* Target : EffectTargets)
		{
			if (!IsValid(Target)) continue;
			TryDispatchPotionEffectToTarget(Target, SlimeTag, HitResult, SourceAbilitySystem, UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target));
		}
	}

	TSet<FGameplayTag> DispatchedTags;
	DispatchedTags.Add(SlimeTag);
	for (const FGameplayTag& EffectTag : PotionEffectTags)
	{
		if (DispatchedTags.Contains(EffectTag)) continue;
		DispatchedTags.Add(EffectTag);
		for (AActor* Target : EffectTargets)
		{
			if (!IsValid(Target)) continue;
			TryDispatchPotionEffectToTarget(Target, EffectTag, HitResult, SourceAbilitySystem, UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target));
		}
	}
}

void UCPPotionImpactComponent::TryDispatchPotionEffectToTarget(AActor* TargetActor, FGameplayTag EffectTag, const FHitResult& HitResult, UAbilitySystemComponent* SourceAbilitySystem, UAbilitySystemComponent* TargetAbilitySystem)
{
	if (!EffectTag.IsValid() || !IsValid(TargetActor) || !IsValid(SourceAbilitySystem) || !IsValid(TargetAbilitySystem)) return;

	FGameplayEffectContextHandle EffectContext = SourceAbilitySystem->MakeEffectContext();
	EffectContext.AddInstigator(ImpactInstigator, GetOwner());
	EffectContext.AddSourceObject(GetOwner());
	EffectContext.AddOrigin(HitResult.ImpactPoint);
	EffectContext.AddHitResult(HitResult, true);

	FGameplayEventData EventData;
	EventData.EventTag = EffectTag;
	EventData.Instigator = ImpactInstigator;
	EventData.Target = TargetActor;
	EventData.OptionalObject = GetOwner();
	EventData.ContextHandle = EffectContext;
	EventData.InstigatorTags = SourceAbilitySystem->GetOwnedGameplayTags();
	EventData.TargetTags = TargetAbilitySystem->GetOwnedGameplayTags();
	EventData.EventMagnitude = 1.0f;

	TargetAbilitySystem->HandleGameplayEvent(EffectTag, &EventData);
}

UAbilitySystemComponent* UCPPotionImpactComponent::ResolveSourceAbilitySystem() const
{
	UAbilitySystemComponent* SourceAbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ImpactInstigator);

	if (!IsValid(SourceAbilitySystem))
	{
		SourceAbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	}

	return SourceAbilitySystem;
}

bool UCPPotionImpactComponent::IsImpactProcessingEnabled() const
{
	return bImpactProcessingEnabled;
}

bool UCPPotionImpactComponent::HasPotionImpactTriggered() const
{
	return bImpactTriggered;
}
