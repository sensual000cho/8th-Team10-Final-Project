#include "Items/Potion/CPPotionImpactComponent.h"

#include "GameFramework/Pawn.h"
#include "Items/Potion/CPPotionImpactContext.h"
#include "Items/Potion/Interface/CPPotionEffectReceiver.h"
#include "Kismet/KismetSystemLibrary.h"
// 이 부분은 포션 액터가 변경되면 변경 예정
#include "Lab/Actor/CPAlchemyProp.h"

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
	if (bImpactTriggered || !IsValid(InInstigator))
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

bool UCPPotionImpactComponent::TryTriggerPotionImpact(const FHitResult& HitResult)
{
	FVector ImpactNormal = HitResult.ImpactNormal.GetSafeNormal();

	if (ImpactNormal.IsNearlyZero())
	{
		ImpactNormal = FVector::UpVector;
	}

	return TryTriggerPotionImpactAtLocation(HitResult.ImpactPoint, ImpactNormal);
}

bool UCPPotionImpactComponent::TryTriggerPotionImpactAtLocation(FVector ImpactPoint, FVector ImpactNormal)
{
	if (!bImpactProcessingEnabled || bImpactTriggered || !IsValid(ImpactInstigator) || !IsValid(GetOwner()) || EffectRadius <= 0.0f)
	{
		return false;
	}

	ImpactNormal = ImpactNormal.GetSafeNormal();

	if (ImpactNormal.IsNearlyZero())
	{
		ImpactNormal = FVector::UpVector;
	}

	// 반복 Hit와 Receiver 재진입보다 먼저 첫 Impact를 확정한다.
	bImpactTriggered = true;
	bImpactProcessingEnabled = false;
	bAnyEffectApplied = ResolvePotionEffectArea(ImpactPoint, ImpactNormal);
	bEffectAreaResolved = true;
	ImpactInstigator = nullptr;

	return true;
}

bool UCPPotionImpactComponent::IsImpactProcessingEnabled() const
{
	return bImpactProcessingEnabled;
}

bool UCPPotionImpactComponent::HasPotionImpactTriggered() const
{
	return bImpactTriggered;
}

bool UCPPotionImpactComponent::IsEffectAreaResolved() const
{
	return bEffectAreaResolved;
}

bool UCPPotionImpactComponent::HasAppliedAnyPotionEffect() const
{
	return bAnyEffectApplied;
}

bool UCPPotionImpactComponent::ResolvePotionEffectArea(const FVector& ImpactPoint, const FVector& ImpactNormal)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());

	TArray<AActor*> OverlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(this, ImpactPoint, EffectRadius, EffectObjectTypes, AActor::StaticClass(), ActorsToIgnore, OverlappedActors);

	TSet<AActor*> ProcessedActors;
	bool bAppliedAnyEffect = false;

	for (AActor* TargetActor : OverlappedActors)
	{
		if (!IsValid(TargetActor) || ProcessedActors.Contains(TargetActor) || !TargetActor->Implements<UCPPotionEffectReceiver>())
		{
			continue;
		}

		ProcessedActors.Add(TargetActor);

		FCPPotionImpactContext Context;

		if (!TryBuildImpactContext(TargetActor, ImpactPoint, ImpactNormal, Context))
		{
			continue;
		}

		if (ICPPotionEffectReceiver::Execute_ReceivePotionImpact(TargetActor, Context))
		{
			bAppliedAnyEffect = true;
		}
	}

	return bAppliedAnyEffect;
}

bool UCPPotionImpactComponent::TryBuildImpactContext(AActor* TargetActor, const FVector& ImpactPoint, const FVector& ImpactNormal, FCPPotionImpactContext& OutContext) const
{
	ACPAlchemyProp* PotionProp = Cast<ACPAlchemyProp>(GetOwner());

	if (!IsValid(PotionProp) || !IsValid(TargetActor) || !IsValid(ImpactInstigator))
	{
		return false;
	}

	const FCPLabIngredientInstance PotionIngredient = PotionProp->GetWorkingIngredient();

	OutContext = FCPPotionImpactContext{};
	OutContext.Instigator = ImpactInstigator;
	OutContext.SourcePotion = PotionProp;
	OutContext.TargetActor = TargetActor;
	OutContext.ImpactPoint = ImpactPoint;
	OutContext.ImpactNormal = ImpactNormal;
	OutContext.EffectTags = PotionIngredient.CurrentEffects;

	return true;
}
