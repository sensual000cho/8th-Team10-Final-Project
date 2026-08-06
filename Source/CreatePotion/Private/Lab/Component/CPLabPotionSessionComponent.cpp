#include "Lab/Component/CPLabPotionSessionComponent.h"

#include "Data/CPForageableItemData.h"
#include "Lab/Actor/CPAlchemyProp.h"

UCPLabPotionSessionComponent::UCPLabPotionSessionComponent()
{
	// 상태가 바뀔 때만 동작하므로 매 프레임 Tick은 사용하지 않음
	PrimaryComponentTick.bCanEverTick = false;
}

FCPLabPotionSessionState UCPLabPotionSessionComponent::GetSessionState() const
{
	return SessionState;
}

bool UCPLabPotionSessionComponent::HasActiveSession() const
{
	return SessionState.IsValid() && SessionState.Phase == ECPLabPotionSessionPhase::InProgress;
}

bool UCPLabPotionSessionComponent::GetActiveRequestState(FCPLabPotionRequestState& OutRequestState) const
{
	return GetRequestState(SessionState.ActiveRequestId,OutRequestState);
}

bool UCPLabPotionSessionComponent::GetRequestState(FName RequestId, FCPLabPotionRequestState& OutRequestState) const
{
	const int32 RequestIndex = FindRequestIndex(RequestId);
	if (RequestIndex == INDEX_NONE){
		OutRequestState = FCPLabPotionRequestState{};
		return false;
	}

	OutRequestState = SessionState.RequestStates[RequestIndex];
	return true;
}

int32 UCPLabPotionSessionComponent::GetDeliveredRequestCount() const
{
	int32 DeliveredCount = 0;
	for (const FCPLabPotionRequestState& RequestState : SessionState.RequestStates){
		if (RequestState.Phase == ECPLabPotionRequestPhase::Delivered) ++DeliveredCount;
	}

	return DeliveredCount;
}

bool UCPLabPotionSessionComponent::StartSession(const TArray<FCPLabPotionRequest>& PotionRequests)
{
	// 진행 중인 세션에는 새 리퀘스트를 덮어쓰지 않음
	if (SessionState.Phase != ECPLabPotionSessionPhase::WaitingForBell ||
		PotionRequests.Num() < CPLabPotionRequestRules::MinRequestCount ||
		PotionRequests.Num() > CPLabPotionRequestRules::MaxRequestCount)
	{
		return false;
	}

	// RequestId는 각 상태를 찾는 키이므로 세션 안에서 중복되면 안 됨
	TSet<FName> RequestIds;
	for (const FCPLabPotionRequest& PotionRequest : PotionRequests)
	{
		if (!PotionRequest.IsValid() || RequestIds.Contains(PotionRequest.RequestId)) return false;

		RequestIds.Add(PotionRequest.RequestId);
	}

	// 입력받은 리퀘스트마다 독립적인 진행 상태와 슬롯 생성
	FCPLabPotionSessionState NewSession;
	NewSession.Phase = ECPLabPotionSessionPhase::InProgress;
	NewSession.RequestStates.Reserve(PotionRequests.Num());

	for (const FCPLabPotionRequest& PotionRequest : PotionRequests){
		FCPLabPotionRequestState RequestState;
		RequestState.PotionRequest = PotionRequest;
		NewSession.RequestStates.Add(MoveTemp(RequestState));
	}

	// 현재 프로토타입은 첫 번째 리퀘스트부터 바로 진행
	NewSession.ActiveRequestId = PotionRequests[0].RequestId;
	SessionState = MoveTemp(NewSession);
	NotifySessionChanged();
	return true;
}

void UCPLabPotionSessionComponent::ResetSession()
{
	SessionState = FCPLabPotionSessionState{};
	NotifySessionChanged();
}

bool UCPLabPotionSessionComponent::TrySetActiveRequest(FName RequestId)
{
	// 이미 선택했거나 납품한 리퀘스트는 다시 활성화하지 않음
	if (!HasActiveSession() || RequestId.IsNone() || SessionState.ActiveRequestId == RequestId) return false;

	const int32 RequestIndex = FindRequestIndex(RequestId);
	if (RequestIndex == INDEX_NONE || 
		SessionState.RequestStates[RequestIndex].Phase == ECPLabPotionRequestPhase::Delivered)
	{
		return false;
	}

	SessionState.ActiveRequestId = RequestId;
	NotifySessionChanged();
	return true;
}

bool UCPLabPotionSessionComponent::TrySetRequestPhase(FName RequestId, ECPLabPotionRequestPhase NewPhase)
{
	const int32 RequestIndex = FindRequestIndex(RequestId);
	if (!HasActiveSession() || RequestIndex == INDEX_NONE || 
		!CanTransitionRequestPhase(SessionState.RequestStates[RequestIndex], NewPhase))
	{
		return false;
	}

	SessionState.RequestStates[RequestIndex].Phase = NewPhase;
	NotifySessionChanged();
	return true;
}

bool UCPLabPotionSessionComponent::TryPlaceIngredient(
	FName RequestId, int32 SlotIndex, ACPAlchemyProp* IngredientProp)
{
	const int32 RequestIndex = FindRequestIndex(RequestId);
	if (!HasActiveSession() || RequestIndex == INDEX_NONE) return false;

	FCPLabPotionRequestState& RequestState = SessionState.RequestStates[RequestIndex];
	if (RequestState.Phase != ECPLabPotionRequestPhase::Preparing || !IsValidSlotIndex(SlotIndex) || 
		!IsValid(IngredientProp) || HasDuplicateIngredient(RequestState, IngredientProp, SlotIndex))
	{
		return false;
	}

	// 슬롯에는 Prop 참조를 저장
	RequestState.IngredientSlots[SlotIndex] = IngredientProp;
	NotifySessionChanged();
	return true;
}

bool UCPLabPotionSessionComponent::TryClearIngredient(FName RequestId, int32 SlotIndex)
{
	const int32 RequestIndex = FindRequestIndex(RequestId);
	if (!HasActiveSession() || RequestIndex == INDEX_NONE) return false;

	FCPLabPotionRequestState& RequestState = SessionState.RequestStates[RequestIndex];
	if (RequestState.Phase != ECPLabPotionRequestPhase::Preparing || !IsValidSlotIndex(SlotIndex) ||
		!IsValid(RequestState.IngredientSlots[SlotIndex]))
	{
		return false;
	}

	RequestState.IngredientSlots[SlotIndex] = nullptr;
	NotifySessionChanged();
	return true;
}

bool UCPLabPotionSessionComponent::TryGetIngredientPropFromSlot(
	FName RequestId, int32 SlotIndex, ACPAlchemyProp*& OutIngredientProp) const
{
	OutIngredientProp = nullptr;
	
	const int32 RequestIndex = FindRequestIndex(RequestId);
	if (!HasActiveSession() || RequestIndex == INDEX_NONE) return false;
	
	const FCPLabPotionRequestState& RequestState = SessionState.RequestStates[RequestIndex];
	if ((RequestState.Phase != ECPLabPotionRequestPhase::Preparing && 
		RequestState.Phase != ECPLabPotionRequestPhase::Processing)) return false;
	if (!IsValidSlotIndex(SlotIndex) || !IsValid(RequestState.IngredientSlots[SlotIndex])) return false;
	
	OutIngredientProp = RequestState.IngredientSlots[SlotIndex];
	return true;
}

bool UCPLabPotionSessionComponent::TryMarkRequestDelivered(FName RequestId)
{
	const int32 RequestIndex = FindRequestIndex(RequestId);
	if (!HasActiveSession() || RequestIndex == INDEX_NONE) return false;

	FCPLabPotionRequestState& RequestState = SessionState.RequestStates[RequestIndex];
	if (RequestState.Phase != ECPLabPotionRequestPhase::PotionReady) return false;

	RequestState.Phase = ECPLabPotionRequestPhase::Delivered;

	// 마지막 리퀘스트면 세션 완료, 아니면 다음 미납품 리퀘스트 선택
	if (AreAllRequestsDelivered()) {
		SessionState.Phase = ECPLabPotionSessionPhase::Completed;
		SessionState.ActiveRequestId = NAME_None;
	}else if (SessionState.ActiveRequestId == RequestId){
		SessionState.ActiveRequestId = FindNextUndeliveredRequestId();
	}

	NotifySessionChanged();
	return true;
}

int32 UCPLabPotionSessionComponent::FindRequestIndex(FName RequestId) const
{
	for (int32 RequestIndex = 0; RequestIndex < SessionState.RequestStates.Num(); ++RequestIndex){
		if (SessionState.RequestStates[RequestIndex].PotionRequest.RequestId == RequestId) return RequestIndex;
	}
	
	return INDEX_NONE;
}

bool UCPLabPotionSessionComponent::IsValidSlotIndex(int32 SlotIndex) const
{
	return SlotIndex >= 0 && SlotIndex < CPLabPotionRequestRules::IngredientSlotCapacity;
}

bool UCPLabPotionSessionComponent::HasDuplicateIngredient(const FCPLabPotionRequestState& RequestState,
	const ACPAlchemyProp* IngredientProp, int32 IgnoredSlotIndex) const
{
	if (!IsValid(IngredientProp)) return false;

	for (int32 SlotIndex = 0; SlotIndex < RequestState.IngredientSlots.Num(); ++SlotIndex){
		// 현재 교체하려는 슬롯은 비교 대상에서 제외
		if (SlotIndex != IgnoredSlotIndex && RequestState.IngredientSlots[SlotIndex] == IngredientProp) return true;
	}

	return false;
}

bool UCPLabPotionSessionComponent::CanTransitionRequestPhase(
	const FCPLabPotionRequestState& RequestState, ECPLabPotionRequestPhase NewPhase) const
{
	// 리퀘스트 상태는 아래 순서로만 한 단계씩 진행
	switch (RequestState.Phase){
	case ECPLabPotionRequestPhase::Queued:
		return NewPhase == ECPLabPotionRequestPhase::Preparing;

	case ECPLabPotionRequestPhase::Preparing:
		return NewPhase == ECPLabPotionRequestPhase::Processing && RequestState.HasValidIngredientSelection();

	case ECPLabPotionRequestPhase::Processing:
		return NewPhase == ECPLabPotionRequestPhase::PotionReady;

	default:
		return false;
	}
}

bool UCPLabPotionSessionComponent::AreAllRequestsDelivered() const
{
	if (SessionState.RequestStates.IsEmpty()) return false;
	
	for (const FCPLabPotionRequestState& RequestState : SessionState.RequestStates){
		if (RequestState.Phase != ECPLabPotionRequestPhase::Delivered) return false;
	}
	
	return true;
}

FName UCPLabPotionSessionComponent::FindNextUndeliveredRequestId() const
{
	for (const FCPLabPotionRequestState& RequestState : SessionState.RequestStates){
		if (RequestState.Phase != ECPLabPotionRequestPhase::Delivered) return RequestState.PotionRequest.RequestId;
	}
	
	return NAME_None;
}

void UCPLabPotionSessionComponent::NotifySessionChanged()
{
	OnSessionChanged.Broadcast();
}
