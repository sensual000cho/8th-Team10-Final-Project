#include "Items/Potion/Debug/CPPotionImpactDummySource.h"

#include "AbilitySystemComponent.h"
#include "Data/CPForageableItemData.h"
#include "GameFramework/Pawn.h"
#include "Items/Potion/CPPotionImpactComponent.h"

ACPPotionImpactDummySource::ACPPotionImpactDummySource()
{
	PotionImpactComponent = CreateDefaultSubobject<UCPPotionImpactComponent>(TEXT("PotionImpactComponent"));
	DebugSourceAbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("DebugSourceAbilitySystemComponent"));
	DebugSourceAbilitySystemComponent->SetIsReplicated(true);
}

UAbilitySystemComponent* ACPPotionImpactDummySource::GetAbilitySystemComponent() const
{
	return DebugSourceAbilitySystemComponent;
}

void ACPPotionImpactDummySource::BeginPlay()
{
	Super::BeginPlay();
	DebugSourceAbilitySystemComponent->InitAbilityActorInfo(this, this);
}

bool ACPPotionImpactDummySource::PrepareDebugPotionImpact(APawn* InInstigator)
{
	if (!IsValid(InInstigator) || !IsValid(TestItemData))
	{
		return false;
	}

	InitializeFromItemData(TestItemData, TestEffectTags);
	// 정식 Potion Actor에서는 손에서 분리되어 실제 투척이 시작된 뒤 Impact 처리를 활성화한다.
	return PotionImpactComponent->EnableImpactProcessing(InInstigator);
}

bool ACPPotionImpactDummySource::TryTriggerDebugPotionImpactAtLocation(FVector ImpactPoint, FVector ImpactNormal)
{
	FHitResult SyntheticHitResult;
	SyntheticHitResult.ImpactPoint = ImpactPoint;
	SyntheticHitResult.Location = ImpactPoint;
	SyntheticHitResult.ImpactNormal = ImpactNormal;
	SyntheticHitResult.Normal = ImpactNormal;
	const bool bTriggered = PotionImpactComponent->TryTriggerPotionImpact(SyntheticHitResult, GetWorkingIngredient().CurrentEffects);

	if (bTriggered)
	{
		ImpactTriggerCount++;
	}

	return bTriggered;
}

void ACPPotionImpactDummySource::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	// 정식 Potion Actor의 Mesh OnHit도 같은 FHitResult를 Impact Component에 전달하면 된다.
	TriggerDebugPotionImpact(Hit);
}

bool ACPPotionImpactDummySource::TriggerDebugPotionImpact(const FHitResult& HitResult)
{
	const bool bTriggered = PotionImpactComponent->TryTriggerPotionImpact(HitResult, GetWorkingIngredient().CurrentEffects);

	if (bTriggered)
	{
		ImpactTriggerCount++;
	}

	return bTriggered;
}
