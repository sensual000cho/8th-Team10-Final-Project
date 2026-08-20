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

bool ACPPotionImpactDummySource::TryApplyDebugPotionImpact(AActor* TargetActor)
{
	if (!IsValid(TargetActor))
	{
		return false;
	}

	FHitResult HitResult;
	HitResult.ImpactPoint = TargetActor->GetActorLocation();
	HitResult.ImpactNormal = (GetActorLocation() - HitResult.ImpactPoint).GetSafeNormal();

	if (HitResult.ImpactNormal.IsNearlyZero())
	{
		HitResult.ImpactNormal = FVector::UpVector;
	}

	return ApplyDebugPotionImpact(TargetActor, HitResult);
}

void ACPPotionImpactDummySource::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	ApplyDebugPotionImpact(Other, Hit);
}

bool ACPPotionImpactDummySource::ApplyDebugPotionImpact(AActor* TargetActor, const FHitResult& HitResult)
{
	ImpactAttemptCount++;

	const bool bApplied = PotionImpactComponent->TryApplyPotionImpact(TargetActor, HitResult);

	if (bApplied)
	{
		SuccessfulImpactCount++;
	}

	return bApplied;
}
