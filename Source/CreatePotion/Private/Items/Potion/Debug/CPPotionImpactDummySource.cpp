#include "Items/Potion/Debug/CPPotionImpactDummySource.h"

#include "Data/CPForageableItemData.h"
#include "GameFramework/Pawn.h"
#include "Items/Potion/CPPotionImpactComponent.h"

ACPPotionImpactDummySource::ACPPotionImpactDummySource()
{
	PotionImpactComponent = CreateDefaultSubobject<UCPPotionImpactComponent>(TEXT("PotionImpactComponent"));
}

bool ACPPotionImpactDummySource::PrepareDebugPotionImpact(APawn* InInstigator)
{
	if (!IsValid(InInstigator) || !IsValid(TestItemData))
	{
		return false;
	}

	InitializeAlchemyProp(TestItemData, TestEffectTags);
	return PotionImpactComponent->EnableImpactProcessing(InInstigator);
}

bool ACPPotionImpactDummySource::TryTriggerDebugPotionImpactAtLocation(FVector ImpactPoint, FVector ImpactNormal)
{
	const bool bTriggered = PotionImpactComponent->TryTriggerPotionImpactAtLocation(ImpactPoint, ImpactNormal);

	if (bTriggered)
	{
		ImpactTriggerCount++;
	}

	return bTriggered;
}

void ACPPotionImpactDummySource::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	TriggerDebugPotionImpact(Hit);
}

bool ACPPotionImpactDummySource::TriggerDebugPotionImpact(const FHitResult& HitResult)
{
	const bool bTriggered = PotionImpactComponent->TryTriggerPotionImpact(HitResult);

	if (bTriggered)
	{
		ImpactTriggerCount++;
	}

	return bTriggered;
}
