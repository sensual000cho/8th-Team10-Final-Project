#include "Lab/Component/CPLabPotionSessionComponent.h"

#include "Data/CPForageableItemData.h"
#include "Lab/Actor/CPAlchemyProp.h"

UCPLabPotionSessionComponent::UCPLabPotionSessionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPLabPotionSessionComponent::ResetPotionResult()
{
	CurrentPotionResult.Reset();
	
	OnSessionChanged.Broadcast();
	OnPotionResultChanged.Broadcast(CurrentPotionResult);
}

bool UCPLabPotionSessionComponent::FinalizePotionResult(ACPAlchemyProp* PotionProp,
	UCPForageableItemData* PotionItemData, const TArray<FGameplayTag>& PotionResult)
{
	if (!IsValid(PotionProp) || !PotionItemData) return false;
	
	CurrentPotionResult = PotionResult;
	PotionProp->InitializeAlchemyProp(PotionItemData, CurrentPotionResult);
	
	OnSessionChanged.Broadcast();
	OnPotionResultChanged.Broadcast(CurrentPotionResult);
	return true;
}


const TArray<FGameplayTag>& UCPLabPotionSessionComponent::GetPotionResult() const
{
	return CurrentPotionResult;
}
