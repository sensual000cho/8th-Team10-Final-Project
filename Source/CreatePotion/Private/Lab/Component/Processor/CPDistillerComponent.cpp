// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Processor/CPDistillerComponent.h"

#include "Lab/Actor/CPAlchemyProp.h"

UCPDistillerComponent::UCPDistillerComponent(): ProcessAmount(1)
{
}

int32 UCPDistillerComponent::CalculateDistilledValue(int32 CurrentValue, int32 AppliedAmount)
{
	if (FMath::Abs(CurrentValue) <= AppliedAmount) return 0;

	return CurrentValue > 0
		? CurrentValue - AppliedAmount
		: CurrentValue + AppliedAmount;
}

bool UCPDistillerComponent::CanProcess(const ACPAlchemyProp* ItemInstance) const
{
	return Super::CanProcess(ItemInstance) && ItemInstance->GetWorkingIngredient().CurrentEffects.Num() > 0;
}

void UCPDistillerComponent::ApplyProcess(ACPAlchemyProp* ItemInstance)
{
	if (!IsValid(ItemInstance)) return;
	
	FCPLabIngredientInstance Ingredient = ItemInstance->GetWorkingIngredient();
	if (!Ingredient.IsValid() || Ingredient.CurrentEffects.IsEmpty()) return;
	
	const int32 AppliedAmount = FMath::RoundToInt(ItemInstance->GetProcessMultiplier() * ProcessAmount);
	
	for (auto It = Ingredient.CurrentEffects.CreateIterator(); It; ++It){
		const int32 NewValue = CalculateDistilledValue(It.Value(), AppliedAmount);

		if (NewValue == 0){
			It.RemoveCurrent();
		}else{
			It.Value() = NewValue;
		}
	}
	
	ItemInstance->SetWorkingIngredient(Ingredient);
	ItemInstance->SetProcessMultiplier(1.f);
}

bool UCPDistillerComponent::BuildPreviewEffects(
	const ACPAlchemyProp* ItemInstance,
	TMap<FGameplayTag, int32>& InOutPreviewEffects) const
{
	if (!IsValid(ItemInstance)) return false;

	const int32 AppliedAmount = FMath::RoundToInt(ItemInstance->GetProcessMultiplier() * ProcessAmount);
	for (TPair<FGameplayTag, int32>& Effect : InOutPreviewEffects){
		Effect.Value = CalculateDistilledValue(Effect.Value, AppliedAmount);
	}

	return true;
}
