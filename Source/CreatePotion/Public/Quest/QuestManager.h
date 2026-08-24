#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuestTypes.h"
#include "QuestManager.generated.h"

// 퀘스트 상태가 바뀔 때마다(수락/완료 등) 방송되는 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestUpdated, FName, QuestID, EQuestState, NewState);

// 완료 보상 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardGranted, int32, GoldAmount);

// 포션 납품 시 판정되는 등급.
// TryDeliver()의 반환값으로 사용되며, 완성된 포션이 퀘스트 조건을
UENUM(BlueprintType)
enum class EDeliveryGrade : uint8
{
	Fail,     // 요구 태그 하나라도 누락 시
	Perfect   // 요구 태그 모두 존재
};

UCLASS()
class CREATEPOTION_API UQuestManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	// 텍스트 전용 DataTable (Row Structure: FQuestData)
	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	UDataTable* QuestScriptTable;

	// 조건/정답 데이터 전용 DataTable (Row Structure: FQuestAnswerData)
	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	UDataTable* QuestAnswerTable;

	//랜덤퀘스트 전용 Datatable
	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	UDataTable* RandomQuestAnswerTable;

	// 퀘스트 상태 변경 알림 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestUpdated OnQuestUpdated;

	// 보상 관련 알림 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnRewardGranted OnRewardGranted;

	// ===================================================================
	// [퀘스트 수락/상태 관리]
	// ===================================================================

	// [수락] NPC 대화 UI에서 "수락" 버튼 클릭 시 호출 / 상태를 Accepted로 전환
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void AcceptQuest(FName QuestID);

	// [상태 조회] 특정 퀘스트가 현재 NotAccepted/Accepted/Completed 중 어디인지 반환
	UFUNCTION(BlueprintCallable, Category = "Quest")
	EQuestState GetQuestState(FName QuestID) const;

	// 저널 UI 목록 구성용 - 현재 추적 중인(수락됨/완료됨) 퀘스트 ID 전체, 수락 순서대로 반환
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FName> GetAllTrackedQuestIDs() const;

	// 퀘스트 완료 처리 함수
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteQuest(FName QuestID);

	// ===================================================================
	// [텍스트 조회 - UI 전용 참조 함수]
	// ===================================================================

	// 저널 목록용 - 짧은 제목
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FText GetQuestTitle(FName QuestID) const;

	// [텍스트-원문] 마을 NPC가 퀘스트를 제안할 때 하는 원문 대사
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FText> GetQuestScriptLines(FName QuestID) const;

	// [텍스트-요약] 수락 후 퀘스트 저널에서 다시 확인할 때 보여줄 요약
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FText GetQuestSummaryText(FName QuestID) const;

	// 저널 UI 등에서 여러 줄 대사를 한 번에(줄바꿈 없이 이어서) 보여줄 때 사용
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FText GetQuestScriptTextJoined(FName QuestID) const;

// ===================================================================
// [세션 힌트 - 단계별 조회 및 현재 단계 자동 관리]
// DT_QuestAnswer에서 값을 가져옴. 실제 판정 수치(RequestedEffects)는 노출하지 않음.
// - GetSessionHintText / Detailed / Detailed2 : 단계별 개별 조회 (저수준)
// - GetCurrentSessionHintText : 저장된 현재 단계에 맞는 힌트를 자동으로 골라 반환 (UI 권장 사용)
// ===================================================================


	// [세션힌트 1차]
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FText GetSessionHintText(FName QuestID) const;

	// 특정 퀘스트의 현재 힌트 단계 조회 - 현재는 스토리용 스크립트 출력용으로 사용
	UFUNCTION(BlueprintCallable, Category = "Quest")
	int32 GetQuestHintLevel(FName QuestID) const;

	// 특정 퀘스트의 힌트 단계 갱신 (저장)
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetQuestHintLevel(FName QuestID, int32 NewLevel);

	// 스크립트 반환용
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FText> GetNPCStoryLines(FName QuestID) const;



	// ===================================================================
	// [납품 판정 - 퍼즐 시스템과의 연결 지점]
	// ===================================================================

	// 퀘스트가 요구하는 조건과 비교, 등급을 매김
	UFUNCTION(BlueprintCallable, Category = "Quest")
	EDeliveryGrade TryDeliver(FName QuestID, const TArray<FGameplayTag>& PotionResult);

	// 조건 하나하나에 대한 세부 판정 (태그정답/태그오답)
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FConditionEvaluation> EvaluateConditions(FName QuestID, const TArray<FGameplayTag>& PotionResult) const;

	// 요구 조건 조회
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FQuestEffectRequirement> GetQuestEffectRequirements(FName QuestID) const;

	// 보상 조회
	UFUNCTION(BlueprintCallable, Category = "Quest")
	int32 GetRewardGold(FName QuestID) const;

	//랜덤 퀘스트 번호 조회
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FName GetRandomQuestID() const;

	// [일반/랜덤 구분] 이 퀘스트가 랜덤 퀘스트 테이블 소속인지 확인
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool IsRandomQuest(FName QuestID) const;

	// 저널 UI - 일반(마을) 퀘스트만 추적 순서대로 반환
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FName> GetTrackedTownQuestIDs() const;

	// 저널 UI - 랜덤(게시판) 퀘스트만 추적 순서대로 반환
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FName> GetTrackedRandomQuestIDs() const;

	// 조건 판정 결과에 맞는 NPC 반응 대사를 랜덤으로 하나 반환
// (EvaluateConditions()가 반환한 FConditionEvaluation을 그대로 넘기면 됨)
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FText GetReactionText(FName QuestID, const FConditionEvaluation& Evaluation) const;

	// ===================================================================
	// [검증 - 개발 중 확인용]
	// ===================================================================

	// [검증] QuestScriptTable과 QuestAnswerTable의 QuestID가 서로 빠짐없이 짝이 맞는지 확인
	// 데이터 입력 실수(한쪽에만 등록)를 개발 중 로그로 잡아내기 위한 함수
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ValidateQuestTablesMatch();


private:
	// 퀘스트별 현재 진행 상태 저장소 (QuestID → 상태)
	UPROPERTY()
	TMap<FName, EQuestState> QuestStates;

	// 퀘스트별 힌트 열람 단계 저장소 (0: 기본, 1: 디테일1, 2: 디테일2)
	UPROPERTY()
	TMap<FName, int32> QuestHintLevels;

	// 수락한 순서를 기록하는 배열
	UPROPERTY()
	TArray<FName> QuestOrder;

	//퀘스트 정답 일치하는지 확인
	// bOutIsRandom이 주어지면, 어느 테이블에서 찾았는지(랜덤 여부)를 같이 알려줌
	FQuestAnswerData* FindAnswerData(FName QuestID, bool* bOutIsRandom = nullptr) const;

	// 반응 텍스트 배열에서 랜덤으로 하나 뽑는 내부 헬퍼 (빈 배열이면 빈 텍스트 반환)
	FText PickRandomReaction(const TArray<FText>& Reactions) const;

protected:
	// Subsystem 생성 시 자동 호출됨. QuestScriptTable/QuestAnswerTable을 자동으로 찾아 연결함.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};

