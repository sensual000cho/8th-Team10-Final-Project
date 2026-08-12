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

	// 입력받은 리퀘스트마다 독립적인 진행 상태 생성
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
	
	// 슬롯 초기화
	IngredientSlots.SetNum(CPLabPotionRequestRules::IngredientSlotCapacity);
	HeldIngredientProp = nullptr;
	CurrentPotionResult.Reset();
	
	NotifySessionChanged();
	return true;
}

void UCPLabPotionSessionComponent::ResetSession()
{
	// Slot에 있는 Delegate 정리
	for (ACPAlchemyProp* IngredientProp : IngredientSlots){
		UnbindIngredientPropChanged(IngredientProp);
	}
	
	SessionState = FCPLabPotionSessionState{};
	IngredientSlots.Reset();
	HeldIngredientProp = nullptr;
	CurrentPotionResult.Reset();
	
	NotifySessionChanged();
	OnPotionResultChanged.Broadcast(CurrentPotionResult);
}

bool UCPLabPotionSessionComponent::TrySetActiveRequest(FName RequestId)
{
	// 이미 선택했거나 납품한 리퀘스트는 다시 활성화하지 않음
	if (!HasActiveSession() || RequestId.IsNone() || SessionState.ActiveRequestId == RequestId) return false;

	const int32 RequestIndex = FindRequestIndex(RequestId);
	if (RequestIndex == INDEX_NONE || 
		SessionState.RequestStates[RequestIndex].Phase == ECPLabPotionRequestPhase::Delivered) return false;

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

bool UCPLabPotionSessionComponent::PlaceIngredient(int32 SlotIndex, ACPAlchemyProp* IngredientProp)
{
	const int32 RequestIndex = FindRequestIndex(SessionState.ActiveRequestId);
	if (!HasActiveSession() || RequestIndex == INDEX_NONE) return false;

	const FCPLabPotionRequestState& RequestState = SessionState.RequestStates[RequestIndex];
	if (RequestState.Phase != ECPLabPotionRequestPhase::Preparing || !IsValidSlotIndex(SlotIndex) || 
		!IsValid(IngredientProp) || HasDuplicateIngredient(IngredientProp, SlotIndex))
	{
		return false;
	}

	ACPAlchemyProp* PreviousIngredientProp = IngredientSlots[SlotIndex];
	UnbindIngredientPropChanged(PreviousIngredientProp);
	
	IngredientSlots[SlotIndex] = IngredientProp;
	BindIngredientPropChanged(IngredientProp);
	
	SyncIngredientActors();
	
	NotifySessionChanged();
	ReBuildResult();
	return true;
}

bool UCPLabPotionSessionComponent::ClearIngredient(int32 SlotIndex)
{
	const int32 RequestIndex = FindRequestIndex(SessionState.ActiveRequestId);
	if (!HasActiveSession() || RequestIndex == INDEX_NONE) return false;

	const FCPLabPotionRequestState& RequestState = SessionState.RequestStates[RequestIndex];
	if (RequestState.Phase != ECPLabPotionRequestPhase::Preparing || !IsValidSlotIndex(SlotIndex) ||
		!IsValid(IngredientSlots[SlotIndex]))
	{
		return false;
	}

	// 현재 슬롯의 이전 delegate 정리
	UnbindIngredientPropChanged(IngredientSlots[SlotIndex]);
	IngredientSlots[SlotIndex] = nullptr;
	
	NotifySessionChanged();
	ReBuildResult();
	return true;
}

bool UCPLabPotionSessionComponent::GetIngredientPropFromSlot(int32 SlotIndex, ACPAlchemyProp*& OutIngredientProp) const
{
	OutIngredientProp = nullptr;
	
	const int32 RequestIndex = FindRequestIndex(SessionState.ActiveRequestId);
	if (!HasActiveSession() || RequestIndex == INDEX_NONE) return false;
	
	const FCPLabPotionRequestState& RequestState = SessionState.RequestStates[RequestIndex];
	if ((RequestState.Phase != ECPLabPotionRequestPhase::Preparing && 
		RequestState.Phase != ECPLabPotionRequestPhase::Processing)) return false;
	if (!IsValidSlotIndex(SlotIndex) || !IsValid(IngredientSlots[SlotIndex])) return false;
	
	OutIngredientProp = IngredientSlots[SlotIndex];
	return true;
}

bool UCPLabPotionSessionComponent::RegisterIngredientSlotActor(int32 SlotIndex, AActor* SlotActor)
{
	if (!IsValidSlotIndex(SlotIndex) || !IsValid(SlotActor)) return false;
	
	if (IngredientSlotActors.Num() != CPLabPotionRequestRules::IngredientSlotCapacity){
		IngredientSlotActors.SetNum(CPLabPotionRequestRules::IngredientSlotCapacity);
	}
	
	IngredientSlotActors[SlotIndex] = SlotActor;
	return true;
}

ACPAlchemyProp* UCPLabPotionSessionComponent::GetHeldIngredientProp() const
{
	return HeldIngredientProp;
}

bool UCPLabPotionSessionComponent::HasHeldIngredient() const
{
	return IsValid(HeldIngredientProp);
}

bool UCPLabPotionSessionComponent::TryHoldIngredient(ACPAlchemyProp* IngredientProp)
{
	if (HasHeldIngredient() || !IsValid(IngredientProp)) return false;
	
	HeldIngredientProp = IngredientProp;
	NotifySessionChanged();
	return true;
}

bool UCPLabPotionSessionComponent::TryReleaseHeldIngredient(ACPAlchemyProp*& OutIngredientProp)
{
	OutIngredientProp = nullptr;
	if (!HasHeldIngredient()) return false;
	
	OutIngredientProp = HeldIngredientProp;
	HeldIngredientProp = nullptr;
	NotifySessionChanged();
	return true;
}

bool UCPLabPotionSessionComponent::FinalizePotionResult(ACPAlchemyProp* PotionProp,	UCPForageableItemData* PotionItemData)
{
	if (!HasActiveSession() || !IsValid(PotionProp) || !PotionItemData) return false;
	
	FCPLabPotionRequestState ActiveRequestState;
	if (!GetActiveRequestState(ActiveRequestState) || 
		ActiveRequestState.Phase != ECPLabPotionRequestPhase::Processing) return false;
	
	PotionProp->InitializeFromEffects(PotionItemData, CurrentPotionResult);
	HeldIngredientProp = PotionProp;
	
	NotifySessionChanged();
	return true;
}

bool UCPLabPotionSessionComponent::InteractIngredientSlot(int32 SlotIndex)
{
	if (!HasActiveSession() || !IsValidSlotIndex(SlotIndex)) return false;
	
	const int32 RequestIndex = FindRequestIndex(SessionState.ActiveRequestId);
	if (RequestIndex == INDEX_NONE) return false;
	
	const FCPLabPotionRequestState& RequestState = SessionState.RequestStates[RequestIndex];
	if (RequestState.Phase != ECPLabPotionRequestPhase::Preparing && 
		RequestState.Phase != ECPLabPotionRequestPhase::Processing) return false;
	
	ACPAlchemyProp* SlotIngredientProp = IngredientSlots[SlotIndex];
	ACPAlchemyProp* CurrentIngredientProp = HeldIngredientProp;
	
	// 빈 손으로 재료가 있는 슬롯을 선택
	if (!IsValid(CurrentIngredientProp)){
		if (!IsValid(SlotIngredientProp)) return false;
		
		HeldIngredientProp = SlotIngredientProp;
		SyncIngredientActors();
		NotifySessionChanged();
		return true;
	}
	
	// 들고 있는 재료의 원래 슬롯을 다시 선택하면 선택 해제
	if (CurrentIngredientProp == SlotIngredientProp){
		HeldIngredientProp = nullptr;
		SyncIngredientActors();
		NotifySessionChanged();
		return true;
	}
	
	const int32 HeldSlotIndex = FindIngredientSlotIndex(CurrentIngredientProp);
	
	if (HeldSlotIndex == INDEX_NONE){
		BindIngredientPropChanged(CurrentIngredientProp);
	}
	
	if (IsValid(SlotIngredientProp) && HeldSlotIndex == INDEX_NONE){
		UnbindIngredientPropChanged(SlotIngredientProp);
	}
	
	if (HeldSlotIndex != INDEX_NONE){
		IngredientSlots[HeldSlotIndex] = SlotIngredientProp;
	}
	
	IngredientSlots[SlotIndex] = CurrentIngredientProp;
	HeldIngredientProp = SlotIngredientProp;
	
	SyncIngredientActors();
	
	NotifySessionChanged();
	ReBuildResult();
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
	ReBuildResult();
	return true;
}

const TArray<FAlchemyProperty>& UCPLabPotionSessionComponent::GetPotionResult() const
{
	return CurrentPotionResult;
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

bool UCPLabPotionSessionComponent::HasDuplicateIngredient(
	const ACPAlchemyProp* IngredientProp, int32 IgnoredSlotIndex) const
{
	if (!IsValid(IngredientProp)) return false;

	for (int32 SlotIndex = 0; SlotIndex < IngredientSlots.Num(); ++SlotIndex){
		// 현재 교체하려는 슬롯은 비교 대상에서 제외
		if (SlotIndex != IgnoredSlotIndex && IngredientSlots[SlotIndex] == IngredientProp) return true;
	}

	return false;
}

int32 UCPLabPotionSessionComponent::FindIngredientSlotIndex(const ACPAlchemyProp* IngredientProp) const
{
	if (!IsValid(IngredientProp)) return INDEX_NONE;
	
	for (int32 SlotIndex = 0; SlotIndex < IngredientSlots.Num(); ++SlotIndex){
		if (IngredientSlots[SlotIndex] == IngredientProp) return SlotIndex;
	}
	
	return INDEX_NONE;
}

void UCPLabPotionSessionComponent::SyncIngredientActors()
{
	for (int32 SlotIndex = 0; SlotIndex < IngredientSlots.Num(); ++SlotIndex){
		ACPAlchemyProp* IngredientProp = IngredientSlots[SlotIndex];
		if (!IsValid(IngredientProp)) continue;
		
		IngredientProp->SetActorHiddenInGame(false);
		IngredientProp->SetActorEnableCollision(true);
		
		const AActor* SlotActor = IngredientSlotActors.IsValidIndex(SlotIndex) 
		? IngredientSlotActors[SlotIndex] : nullptr;
		
		MoveIngredientPropToSlot(IngredientProp, SlotActor);
	}
	
	if (IsValid(HeldIngredientProp) && FindIngredientSlotIndex(HeldIngredientProp) == INDEX_NONE){
		HeldIngredientProp->SetActorHiddenInGame(true);
		HeldIngredientProp->SetActorEnableCollision(false);
	}
}

void UCPLabPotionSessionComponent::MoveIngredientPropToSlot(ACPAlchemyProp* IngredientProp, const AActor* SlotActor)
{
	if (!IsValid(IngredientProp) || !IsValid(SlotActor)) return;
	
	IngredientProp->SetActorTransform(SlotActor->GetActorTransform());
	
	// Slot 액터와 재료 Actor의 실제 Bounds 가져옴
	FVector SlotOrigin, SlotExtent;
	FVector PropOrigin, PropExtent;
	SlotActor->GetActorBounds(false, SlotOrigin, SlotExtent);
	IngredientProp->GetActorBounds(false, PropOrigin, PropExtent);
	
	// 재료가 파묻치지 않기 위한 z값 계산
	const float SlotSurfaceZ = SlotOrigin.Z + SlotExtent.Z;
	const float PropBottemZ = PropOrigin.Z - PropExtent.Z;
	const float ZOffset = SlotSurfaceZ - PropBottemZ;
	
	// 높이 보정
	IngredientProp->AddActorWorldOffset(FVector(0.f, 0.f, ZOffset), false);
}

int32 UCPLabPotionSessionComponent::GetSelectedIngredientCount() const
{
	int32 SelectedCount = 0;
	for (const ACPAlchemyProp* IngredientProp : IngredientSlots){
		if (IsValid(IngredientProp)) ++SelectedCount;
	}
	
	return SelectedCount;
}

bool UCPLabPotionSessionComponent::HasValidIngredientSelection() const
{
	const int32 SelectedCounted = GetSelectedIngredientCount();
	return SelectedCounted >= CPLabPotionRequestRules::MinSelectedIngredientCount &&
		SelectedCounted <= CPLabPotionRequestRules::MaxSelectedIngredientCount;
}

void UCPLabPotionSessionComponent::BindIngredientPropChanged(ACPAlchemyProp* IngredientProp)
{
	if (!IsValid(IngredientProp)) return;
	
	IngredientProp->OnAlchemyPropChanged.RemoveDynamic(
		this, &UCPLabPotionSessionComponent::HandleIngredientPropChanged);
	IngredientProp->OnAlchemyPropChanged.AddDynamic(
		this, &UCPLabPotionSessionComponent::HandleIngredientPropChanged);
}

void UCPLabPotionSessionComponent::UnbindIngredientPropChanged(ACPAlchemyProp* IngredientProp)
{
	if (!IsValid(IngredientProp)) return;
	
	IngredientProp->OnAlchemyPropChanged.RemoveDynamic(
		this, &UCPLabPotionSessionComponent::HandleIngredientPropChanged);
}

bool UCPLabPotionSessionComponent::CanTransitionRequestPhase(
	const FCPLabPotionRequestState& RequestState, ECPLabPotionRequestPhase NewPhase) const
{
	// 리퀘스트 상태는 아래 순서로만 한 단계씩 진행
	switch (RequestState.Phase){
	case ECPLabPotionRequestPhase::Queued:
		return NewPhase == ECPLabPotionRequestPhase::Preparing;

	case ECPLabPotionRequestPhase::Preparing:
		return NewPhase == ECPLabPotionRequestPhase::Processing && HasValidIngredientSelection();

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

void UCPLabPotionSessionComponent::HandleIngredientPropChanged()
{
	ReBuildResult();
}

void UCPLabPotionSessionComponent::ReBuildResult()
{
	TMap<FGameplayTag, int32> EffectTotalMap;
	
	// Map에 Key: Tag, Value: Value의 형태로 저장
	for (ACPAlchemyProp* IngredientProp : IngredientSlots){
		if (!IsValid(IngredientProp)) continue;
		
		const FCPLabIngredientInstance Ingredient = IngredientProp->GetWorkingIngredient();
		if (!Ingredient.IsValid()) continue;
		
		for (const TPair<FGameplayTag, int32>& Effect : Ingredient.CurrentEffects){
			if (!Effect.Key.IsValid()) continue;
			
			EffectTotalMap.FindOrAdd(Effect.Key) += Effect.Value;
		}
	}
	
	// 이전 결과 정리 및 Map을 Array로 변경
	CurrentPotionResult.Reset();
	CurrentPotionResult.Reserve(EffectTotalMap.Num());
	
	for (const TPair<FGameplayTag, int32>& Effect : EffectTotalMap){
		if (Effect.Value == 0) continue;
		FAlchemyProperty EffectTotal;
		EffectTotal.Tag = Effect.Key;
		EffectTotal.Value = Effect.Value;
		CurrentPotionResult.Add(EffectTotal);
	}
	
	OnPotionResultChanged.Broadcast(CurrentPotionResult);
}

void UCPLabPotionSessionComponent::NotifySessionChanged()
{
	OnSessionChanged.Broadcast();
}
