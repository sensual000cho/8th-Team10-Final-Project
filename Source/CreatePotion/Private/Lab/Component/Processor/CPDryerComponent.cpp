// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Processor/CPDryerComponent.h"

#include "Lab/Actor/CPAlchemyProp.h"

UCPDryerComponent::UCPDryerComponent(): ProcessAmount(1)
{
}

int32 UCPDryerComponent::CalculateDriedValue(int32 CurrentValue, int32 AppliedAmount)
{
	return CurrentValue > 0
		? CurrentValue + AppliedAmount
		: CurrentValue - AppliedAmount;
}

bool UCPDryerComponent::CanProcess(const ACPAlchemyProp* ItemInstance) const
{
	return Super::CanProcess(ItemInstance) && ItemInstance->GetWorkingIngredient().CurrentEffects.Num() > 0;
}

void UCPDryerComponent::ApplyProcess(ACPAlchemyProp* ItemInstance)
{
	if (!IsValid(ItemInstance)) return;
	
	FCPLabIngredientInstance Ingredient = ItemInstance->GetWorkingIngredient();
	if (!Ingredient.IsValid() || Ingredient.CurrentEffects.IsEmpty()) return;
	
	const int32 AppliedAmount = FMath::RoundToInt(ItemInstance->GetProcessMultiplier() * ProcessAmount);
	
	// 절댓값으로 증가
	for (TPair<FGameplayTag, int32>& Effect : Ingredient.CurrentEffects){
		Effect.Value = CalculateDriedValue(Effect.Value, AppliedAmount);
	}
	
	// Prop에 사용한 기구로 처리
	ItemInstance->SetWorkingIngredient(Ingredient);
	ItemInstance->SetProcessMultiplier(1.f);
}

bool UCPDryerComponent::BuildPreviewEffects(
	const ACPAlchemyProp* ItemInstance,
	TMap<FGameplayTag, int32>& InOutPreviewEffects) const
{
	if (!IsValid(ItemInstance)) return false;

	const int32 AppliedAmount = FMath::RoundToInt(ItemInstance->GetProcessMultiplier() * ProcessAmount);
	for (TPair<FGameplayTag, int32>& Effect : InOutPreviewEffects){
		Effect.Value = CalculateDriedValue(Effect.Value, AppliedAmount);
	}

	return true;
}
