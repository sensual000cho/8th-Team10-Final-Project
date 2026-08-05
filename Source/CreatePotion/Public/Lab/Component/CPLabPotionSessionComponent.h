#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Lab/CPLabPotionRequestTypes.h"
#include "CPLabPotionSessionComponent.generated.h"

class UCPForageableItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCPOnLabSessionChanged);

// 포션 세션과 리퀘스트별 슬롯 상태를 한곳에서 관리
UCLASS()
class CREATEPOTION_API UCPLabPotionSessionComponent
	: public UActorComponent
{
	GENERATED_BODY()

public:
	UCPLabPotionSessionComponent();

	UFUNCTION(BlueprintPure, Category = "Lab|Session")
	FCPLabPotionSessionState GetSessionState() const;

	UFUNCTION(BlueprintPure, Category = "Lab|Session")
	bool HasActiveSession() const;

	UFUNCTION(BlueprintPure, Category = "Lab|Request")
	bool GetActiveRequestState(
		FCPLabPotionRequestState& OutRequestState) const;

	UFUNCTION(BlueprintPure, Category = "Lab|Request")
	bool GetRequestState(
		FName RequestId,
		FCPLabPotionRequestState& OutRequestState) const;

	UFUNCTION(BlueprintPure, Category = "Lab|Request")
	int32 GetDeliveredRequestCount() const;

	// 전달받은 리퀘스트들로 새 포션 세션 시작
	bool StartSession(
		const TArray<FCPLabPotionRequest>& PotionRequests);

	// 진행 중인 세션과 슬롯을 모두 초기 상태로 되돌림
	void ResetSession();

	// 플레이어가 진행할 리퀘스트를 RequestId로 선택
	bool TrySetActiveRequest(FName RequestId);

	// 정해진 진행 순서에 맞을 때만 리퀘스트 상태 변경
	bool TrySetRequestPhase(
		FName RequestId,
		ECPLabPotionRequestPhase NewPhase);

	// 지정한 리퀘스트의 슬롯에 재료 확정
	bool TryPlaceIngredient(
		FName RequestId,
		int32 SlotIndex,
		const FCPLabIngredientInstance& Ingredient);

	// 지정한 슬롯에 확정된 재료 제거
	bool TryClearIngredient(
		FName RequestId,
		int32 SlotIndex);

	// 슬롯의 확정 재료를 가공용 작업본으로 복사
	bool TryCreateWorkingCopy(
		FName RequestId,
		int32 SlotIndex,
		FCPLabIngredientInstance& OutWorkingCopy) const;

	// 가공이 끝난 작업본을 원래 슬롯에 다시 반영
	bool TryCommitWorkingCopy(
		FName RequestId,
		int32 SlotIndex,
		const FCPLabIngredientInstance& WorkingCopy);

	// 완성된 포션을 납품 처리하고 필요하면 세션 완료
	bool TryMarkRequestDelivered(FName RequestId);

	// 세션이나 슬롯 상태가 바뀌었음을 Blueprint에 알림
	UPROPERTY(BlueprintAssignable, Category = "Lab|Session")
	FCPOnLabSessionChanged OnSessionChanged;

private:
	// RequestId와 일치하는 상태의 배열 위치 찾기
	int32 FindRequestIndex(FName RequestId) const;

	// SlotIndex가 실제 슬롯 배열 범위 안인지 확인
	bool IsValidSlotIndex(int32 SlotIndex) const;

	// 같은 리퀘스트의 다른 슬롯에 동일한 원본 재료가 있는지 확인
	bool HasDuplicateIngredient(
		const FCPLabPotionRequestState& RequestState,
		const UCPForageableItemData* ItemData,
		int32 IgnoredSlotIndex) const;

	// 현재 상태에서 요청한 다음 상태로 이동할 수 있는지 확인
	bool CanTransitionRequestPhase(
		const FCPLabPotionRequestState& RequestState,
		ECPLabPotionRequestPhase NewPhase) const;

	// 세션의 모든 리퀘스트가 납품됐는지 확인
	bool AreAllRequestsDelivered() const;

	// 아직 납품하지 않은 다음 리퀘스트 ID 찾기
	FName FindNextUndeliveredRequestId() const;

	// 상태 변경 Delegate를 한곳에서 호출
	void NotifySessionChanged();

	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Session")
	FCPLabPotionSessionState SessionState;
};
