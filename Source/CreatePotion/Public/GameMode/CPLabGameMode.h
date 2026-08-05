#pragma once

#include "CoreMinimal.h"
#include "GameMode/CPGameModeBase.h"
#include "Lab/CPLabPotionRequestTypes.h"
#include "CPLabGameMode.generated.h"

class ACPLabGameState;
class UCPLabPotionSessionComponent;

// Blueprint와 월드 Actor가 포션 세션 기능을 호출하는 입구
UCLASS()
class CREATEPOTION_API ACPLabGameMode : public ACPGameModeBase
{
	GENERATED_BODY()

public:
	ACPLabGameMode();

	// 현재 설정된 테스트 리퀘스트로 세션 시작
	UFUNCTION(BlueprintCallable, Category = "Lab|Session")
	bool TryStartLabSession();

	// 외부에서 전달받은 리퀘스트 목록으로 세션 시작
	UFUNCTION(BlueprintCallable, Category = "Lab|Session")
	bool TryStartLabSessionWithRequests(
		const TArray<FCPLabPotionRequest>& PotionRequests);

	// 진행 중인 포션 세션을 벨 대기 상태로 초기화
	UFUNCTION(BlueprintCallable, Category = "Lab|Session")
	void ResetLabSession();

	// 여러 리퀘스트 중 플레이어가 진행할 리퀘스트 선택
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TrySelectRequest(FName RequestId);

	// 활성 리퀘스트를 수락하고 재료 준비 상태로 전환
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TryAcceptActiveRequest();

	// 준비한 재료가 유효하면 가공 상태로 전환
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TryBeginActiveRequestProcessing();

	// 가공 중인 리퀘스트를 포션 완성 상태로 전환
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TryFinishActivePotion();

	// 완성된 포션을 납품 처리
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TryDeliverActivePotion();

	// 지정한 리퀘스트 슬롯에 재료 배치 요청
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TryPlaceIngredient(
		FName RequestId,
		int32 SlotIndex,
		const FCPLabIngredientInstance& Ingredient);

	// 지정한 리퀘스트 슬롯의 재료 제거 요청
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TryClearIngredient(
		FName RequestId,
		int32 SlotIndex);

	// 슬롯 재료를 가공 Actor가 사용할 작업본으로 가져오기
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TryCreateWorkingIngredient(
		FName RequestId,
		int32 SlotIndex,
		FCPLabIngredientInstance& OutWorkingIngredient) const;

	// 가공이 끝난 작업본을 지정한 슬롯에 반영
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TryCommitWorkingIngredient(
		FName RequestId,
		int32 SlotIndex,
		const FCPLabIngredientInstance& WorkingIngredient);

	// 현재 상태를 다음 단계로 넘기는 테스트 전용 함수
	UFUNCTION(BlueprintCallable, Category = "Lab|Debug")
	void DebugAdvanceSessionPhase();

private:
	ACPLabGameState* GetLabGameState() const;
	UCPLabPotionSessionComponent* GetPotionSession() const;
	FName GetActiveRequestId() const;

	// 실제 리퀘스트 시스템이 연결되기 전 사용할 테스트 데이터
	UPROPERTY(EditDefaultsOnly, Category = "Lab|Debug")
	TArray<FCPLabPotionRequest> DefaultTestRequests;
};
