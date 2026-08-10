// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Processor/CPDryerComponent.h"

#include "Lab/Actor/CPAlchemyProp.h"

UCPDryerComponent::UCPDryerComponent(): ProcessAmount(1)
{
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
		if (Effect.Value > 0) Effect.Value += AppliedAmount;
		else Effect.Value -= AppliedAmount;
	}
	
	// Prop에 사용한 기구로 처리
	ItemInstance->SetWorkingIngredient(Ingredient);
	ItemInstance->SetProcessMultiplier(1.f);
}
