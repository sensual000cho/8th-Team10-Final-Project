#include "Lab/Component/CPLabPotionSessionComponent.h"

#include "Data/CPForageableItemData.h"
#include "Lab/Actor/CPAlchemyProp.h"

UCPLabPotionSessionComponent::UCPLabPotionSessionComponent() : bHasActiveRequest(false)
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UCPLabPotionSessionComponent::HasActiveRequest() const
{
	return bHasActiveRequest && ActiveRequestState.IsValid();
}

bool UCPLabPotionSessionComponent::GetActiveRequestState(FCPLabPotionRequestState& OutRequestState) const
{
	if (!HasActiveRequest()){
		OutRequestState = FCPLabPotionRequestState{};
		return false;
	}
	
	OutRequestState = ActiveRequestState;
	return true;
}

bool UCPLabPotionSessionComponent::StartRequest(const FCPLabPotionRequest& PotionRequest)
{
	if (HasActiveRequest() || !PotionRequest.IsValid()) return false;
	
	ActiveRequestState = FCPLabPotionRequestState{};
	ActiveRequestState.PotionRequest = PotionRequest;
	bHasActiveRequest = true;
	
	HeldAlchemyProp = nullptr;
	CurrentPotionResult.Reset();
	
	OnSessionChanged.Broadcast();
	OnPotionResultChanged.Broadcast(CurrentPotionResult);
	return true;
}

void UCPLabPotionSessionComponent::ResetRequest()
{
	ActiveRequestState = FCPLabPotionRequestState{};
	bHasActiveRequest = false;
	
	HeldAlchemyProp = nullptr;
	CurrentPotionResult.Reset();
	
	OnSessionChanged.Broadcast();
	OnPotionResultChanged.Broadcast(CurrentPotionResult);
}

ACPAlchemyProp* UCPLabPotionSessionComponent::GetHeldAlchemyProp() const
{
	return HeldAlchemyProp;
}

bool UCPLabPotionSessionComponent::HasHeldAlchemyProp() const
{
	return IsValid(HeldAlchemyProp);
}

bool UCPLabPotionSessionComponent::HoldAlchemyProp(ACPAlchemyProp* AlchemyProp)
{
	if (HasHeldAlchemyProp() || !IsValid(AlchemyProp)) return false;
	
	HeldAlchemyProp = AlchemyProp;
	OnHeldAlchemyPropChanged.Broadcast();
	return true;
}

bool UCPLabPotionSessionComponent::ReleaseHeldAlchemyProp(ACPAlchemyProp*& OutAlchemyProp)
{
	OutAlchemyProp = nullptr;
	if (!HasHeldAlchemyProp()) return false;
	
	OutAlchemyProp = HeldAlchemyProp;
	HeldAlchemyProp = nullptr;
	OnHeldAlchemyPropChanged.Broadcast();
	return true;
}

bool UCPLabPotionSessionComponent::FinalizePotionResult(ACPAlchemyProp* PotionProp,
	UCPForageableItemData* PotionItemData, const TArray<FGameplayTag>& PotionResult)
{
	if (!IsValid(PotionProp) || !PotionItemData) return false;
	
	CurrentPotionResult = PotionResult;
	PotionProp->InitializeAlchemyProp(PotionItemData, CurrentPotionResult);
	
	OnSessionChanged.Broadcast();
	return true;
}


const TArray<FGameplayTag>& UCPLabPotionSessionComponent::GetPotionResult() const
{
	return CurrentPotionResult;
}
