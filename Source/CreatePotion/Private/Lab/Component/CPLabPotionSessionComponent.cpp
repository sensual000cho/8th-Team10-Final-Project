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
	ActiveRequestState.Phase = ECPLabPotionRequestPhase::Processing;
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

bool UCPLabPotionSessionComponent::SetRequestPhase(ECPLabPotionRequestPhase NewPhase)
{
	if (!HasActiveRequest() || !CanTransitionRequestPhase(NewPhase)) return false;

	ActiveRequestState.Phase = NewPhase;
	OnSessionChanged.Broadcast();
	return true;
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
	// 다른 델리게이트로 변경 예정
	//OnSessionChanged.Broadcast();
	return true;
}

bool UCPLabPotionSessionComponent::ReleaseHeldAlchemyProp(ACPAlchemyProp*& OutAlchemyProp)
{
	OutAlchemyProp = nullptr;
	if (!HasHeldAlchemyProp()) return false;
	
	OutAlchemyProp = HeldAlchemyProp;
	HeldAlchemyProp = nullptr;
	// 다른 델리게이트로 변경 예정
	//OnSessionChanged.Broadcast();
	return true;
}

bool UCPLabPotionSessionComponent::FinalizePotionResult(ACPAlchemyProp* PotionProp,	UCPForageableItemData* PotionItemData)
{
	if (!HasActiveRequest() || !IsValid(PotionProp) || !PotionItemData) return false;
	if (ActiveRequestState.Phase != ECPLabPotionRequestPhase::Processing) return false;
	
	PotionProp->InitializeAlchemyProp(PotionItemData, CurrentPotionResult);
	HeldAlchemyProp = PotionProp;
	
	OnSessionChanged.Broadcast();
	return true;
}

const TArray<FGameplayTag>& UCPLabPotionSessionComponent::GetPotionResult() const
{
	return CurrentPotionResult;
}

bool UCPLabPotionSessionComponent::CanTransitionRequestPhase(ECPLabPotionRequestPhase NewPhase) const
{
	if (!HasActiveRequest()) return false;
	
	// 리퀘스트 상태는 아래 순서로만 한 단계씩 진행
	switch (ActiveRequestState.Phase){
	case ECPLabPotionRequestPhase::Processing:
		return NewPhase == ECPLabPotionRequestPhase::PotionReady;

	default:
		return false;
	}
}
