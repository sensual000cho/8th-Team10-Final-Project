// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Processor/CPDistillerComponent.h"

#include "Lab/Actor/CPAlchemyProp.h"

UCPDistillerComponent::UCPDistillerComponent(): ProcessAmount(1)
{
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
		const int32 CurrentValue = It.Value();
		
		if (FMath::Abs(CurrentValue) <= AppliedAmount){
			It.RemoveCurrent();
		}else if (CurrentValue > 0){
			It.Value() -= AppliedAmount;
		}else{
			It.Value() += AppliedAmount;
		}
	}
	
	ItemInstance->SetWorkingIngredient(Ingredient);
	ItemInstance->SetProcessMultiplier(1.f);
}
