#include "Items/Potion/CPPotionImpactComponent.h"
#include "GameFramework/Pawn.h"
#include "Items/Potion/CPPotionImpactContext.h"
#include "Items/Potion/Interface/CPPotionEffectReceiver.h"
//이 부분은 포션 액터가 변경되면 변경 예정
#include "Lab/Actor/CPAlchemyProp.h"

UCPPotionImpactComponent::UCPPotionImpactComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UCPPotionImpactComponent::EnableImpactProcessing(APawn* InInstigator)
{
	if (bConsumed || !IsValid(InInstigator))
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

bool UCPPotionImpactComponent::TryApplyPotionImpact(AActor* TargetActor, const FHitResult& HitResult)
{
	// 투척 전 상태, 이미 소비된 상태, Receiver 처리 중 재진입을 차단
	if (!bImpactProcessingEnabled || bConsumed || bApplyingImpact)
	{
		return false;
	}

	//자기 자신, Invalid Actor 무시
	if (!IsValid(TargetActor) || TargetActor == GetOwner())
	{
		return false;
	}

	// 대상 Actor가 포션 Receiver 인터페이스를 구현하지 않았으면 무시
	if (!TargetActor->Implements<UCPPotionEffectReceiver>())
	{
		return false;
	}

	FCPPotionImpactContext Context;

	if (!TryBuildImpactContext(TargetActor, HitResult, Context))
	{
		return false;
	}

	//여기서 대상한테 효과 적용
	bApplyingImpact = true;
	const bool bApplied = ICPPotionEffectReceiver::Execute_ReceivePotionImpact(TargetActor, Context);
	bApplyingImpact = false;

	
	// Receiver가 실제 효과를 하나 이상 적용하지 못했다면 소비하지 않음
	if (!bApplied)
	{
		return false;
	}

	bConsumed = true;
	DisableImpactProcessing();

	return true;

}

bool UCPPotionImpactComponent::IsImpactProcessingEnabled() const
{
	return bImpactProcessingEnabled;
}

bool UCPPotionImpactComponent::IsPotionConsumed() const
{
	return bConsumed;
}

bool UCPPotionImpactComponent::TryBuildImpactContext(AActor* TargetActor, const FHitResult& HitResult, FCPPotionImpactContext& OutContext) const
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
	OutContext.ImpactPoint = HitResult.ImpactPoint;
	OutContext.ImpactNormal = HitResult.ImpactNormal;
	OutContext.EffectTags = PotionIngredient.CurrentEffects;

	return true;
}
