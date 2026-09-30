#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuestTypes.h"
#include "QuestManager.generated.h"

// 퀘스트 상태 변경(수락/완료 등) 시 방송
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestUpdated, FName, QuestID, EQuestState, NewState);

// 완료 보상 지급 시 방송
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardGranted, int32, GoldAmount);

USTRUCT()

struct FCPQuestSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FName, EQuestState> QuestStates;

	UPROPERTY()
	TArray<FName> QuestResults;

	UPROPERTY()
	TMap<FName, int32> NPCStoryStages;

	UPROPERTY()
	FName PinnedQuestID = NAME_None;
};

UCLASS()
class CREATEPOTION_API UQuestManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//Game Save
	FCPQuestSaveData GetSaveData();
	void LoadSaveData(const FCPQuestSaveData& SaveData);
	// ===================================================================
	// [데이터 테이블] - Initialize()에서 자동 연결
	// ===================================================================

	// 텍스트 (Row: FQuestData)
	UPROPERTY()
	TObjectPtr<UDataTable> QuestScriptTable;

	// 조건/정답 (Row: FQuestAnswerData)
	UPROPERTY()
	TObjectPtr<UDataTable> QuestAnswerTable;

	// 랜덤 퀘스트 조건/정답 (Row: FQuestAnswerData)
	UPROPERTY()
	TObjectPtr<UDataTable> RandomQuestAnswerTable;

	// ===================================================================
	// [이벤트]
	// ===================================================================

	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestUpdated OnQuestUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnRewardGranted OnRewardGranted;

	// ===================================================================
	// [상태 관리]
	// ===================================================================

	// 수락 - 상태를 Accepted로 전환
	UFUNCTION(BlueprintCallable, Category = "Quest|State")
	void AcceptQuest(FName QuestID);

	// 완료 처리
	UFUNCTION(BlueprintCallable, Category = "Quest|State")
	void CompleteQuest(FName QuestID);

	// NotAccepted / Accepted / Completed 조회
	UFUNCTION(BlueprintCallable, Category = "Quest|State")
	EQuestState GetQuestState(FName QuestID) const;

	// 랜덤 퀘스트 테이블 소속 여부
	UFUNCTION(BlueprintCallable, Category = "Quest|State")
	bool IsRandomQuest(FName QuestID) const;

	// 랜덤 퀘스트 ID 하나 선택
	UFUNCTION(BlueprintCallable, Category = "Quest|State")
	FName GetRandomQuestID() const;

	// ===================================================================
	// [저널 - 추적 목록] 모두 수락 순서대로 반환
	// ===================================================================

	// 전체 (수락됨/완료됨)
	UFUNCTION(BlueprintCallable, Category = "Quest|Journal")
	TArray<FName> GetAllTrackedQuestIDs() const;

	// 일반(마을) 퀘스트만
	UFUNCTION(BlueprintCallable, Category = "Quest|Journal")
	TArray<FName> GetTrackedTownQuestIDs() const;

	// 랜덤(게시판) 퀘스트만
	UFUNCTION(BlueprintCallable, Category = "Quest|Journal")
	TArray<FName> GetTrackedRandomQuestIDs() const;

	// ===================================================================
	// [목표 UI / 마커]
	// ===================================================================

	// 현재 활성 목표
	UFUNCTION(BlueprintCallable, Category = "Quest|Goal")
	FName GetActiveGoalQuestID() const;

	// 목표 UI에 고정 표시할 퀘스트
	UFUNCTION(BlueprintCallable, Category = "Quest|Goal")
	void SetPinnedQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest|Goal")
	FName GetPinnedQuest() const;

	// NPC가 가진 퀘스트들의 상태를 종합한 마커 상태
	UFUNCTION(BlueprintCallable, Category = "Quest|Goal")
	EQuestMarkerState GetMarkerStateForQuests(const TArray<FName>& QuestIDs, const TMap<FName, int32>& HeldItemCounts) const;

	// ===================================================================
	// [텍스트 조회]
	// ===================================================================

	// 저널 목록용 짧은 제목
	UFUNCTION(BlueprintCallable, Category = "Quest|Text")
	FText GetQuestTitle(FName QuestID) const;

	// NPC 제안 원문 대사
	UFUNCTION(BlueprintCallable, Category = "Quest|Text")
	TArray<FText> GetQuestScriptLines(FName QuestID) const;

	// 원문 대사를 한 줄로 이어 붙인 버전
	UFUNCTION(BlueprintCallable, Category = "Quest|Text")
	FText GetQuestScriptTextJoined(FName QuestID) const;

	// 수락 후 저널 요약
	UFUNCTION(BlueprintCallable, Category = "Quest|Text")
	FText GetQuestSummaryText(FName QuestID) const;

	// 인물 도감 요약
	UFUNCTION(BlueprintCallable, Category = "Quest|Text")
	FText GetCodexSummaryText(FName QuestID) const;

	// 세션 힌트 (판정 수치는 노출하지 않음)
	UFUNCTION(BlueprintCallable, Category = "Quest|Text")
	FText GetSessionHintText(FName QuestID) const;

	// ===================================================================
	// [스토리 진행] - NPC별 독립 진행도
	// ===================================================================

	// NPC 스토리 현재 단계 (기록 없으면 0)
	UFUNCTION(BlueprintCallable, Category = "Quest|Story")
	int32 GetNPCStoryStage(FName NPCId) const;

	// 해당 NPC 스토리 단계 기준으로 지금 노출 가능한지
	UFUNCTION(BlueprintCallable, Category = "Quest|Story")
	bool IsQuestStoryUnlocked(FName QuestID) const;

	// 스토리 스크립트 반환
	UFUNCTION(BlueprintCallable, Category = "Quest|Story")
	TArray<FText> GetNPCStoryLines(FName QuestID) const;

	// 퀘스트별 스크립트 출력 단계 조회/갱신
	UFUNCTION(BlueprintCallable, Category = "Quest|Story")
	int32 GetQuestHintLevel(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Story")
	void SetQuestHintLevel(FName QuestID, int32 NewLevel);

	
	// [납품 판정]
	

	// 요구 조건과 비교해 Perfect면 완료 처리
	// ※ 포션 납품 시에는 직접 호출하지 말고 CreateResult()를 사용 (내부에서 호출함)
	UFUNCTION(BlueprintCallable, Category = "Quest|Delivery")
	EDeliveryGrade TryDeliver(FName QuestID, const TArray<FGameplayTag>& PotionResult);

	// 조건별 세부 판정
	UFUNCTION(BlueprintCallable, Category = "Quest|Delivery")
	TArray<FConditionEvaluation> EvaluateConditions(FName QuestID, const TArray<FGameplayTag>& PotionResult) const;

	// 판정 결과에 맞는 NPC 반응 대사 (랜덤 1개)
	UFUNCTION(BlueprintCallable, Category = "Quest|Delivery")
	FText GetReactionText(FName QuestID, const FConditionEvaluation& Evaluation) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Delivery")
	TArray<FQuestEffectRequirement> GetQuestEffectRequirements(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Delivery")
	int32 GetRewardGold(FName QuestID) const;

	// ===================================================================
	// [결과]

	// 결과 생성 (C++ 전용)
	// - Potion형: 내부에서 TryDeliver()로 판정·완료까지 처리 → TryDeliver() 중복 호출 금지
	// - 그 외: 완료 함수(CompleteQuestByDialogue 등) 내부에서 자동 호출됨
	FCPQuestResult CreateResult(FName QuestID, const TArray<FGameplayTag>& PotionResult = TArray<FGameplayTag>());

	UFUNCTION(BlueprintCallable, Category = "Quest|Result")
	FCPQuestResult GetResult(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Result")
	bool ClearResult();

	// ===================================================================
	// [완료 방식별 트리거] - 포션 납품 외 경로

	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	EQuestCompletionType GetQuestCompletionType(FName QuestID) const;

	// [Dialogue]
	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	bool CompleteQuestByDialogue(FName QuestID);

	// [FindTarget]
	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	bool TryCompleteQuestByFindTarget(FName QuestID, FName FoundTargetId);

	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	FName GetTargetIdentifier(FName QuestID) const;

	// [ItemCollection]
	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	bool TryCompleteQuestByItem(FName QuestID, const TMap<FName, int32>& HeldItemCounts);

	// 판정 없이 완료 가능 여부만 조회
	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	bool IsItemQuestCompletable(FName QuestID, const TMap<FName, int32>& HeldItemCounts) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	void GetRequiredItemInfo(FName QuestID, TArray<FName>& OutItemIDs, int32& OutRequiredCount) const;

	// "수상한 재료: 1/2" 형태
	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	FText GetItemProgressText(FName QuestID, const TMap<FName, int32>& HeldItemCounts) const;

	// ===================================================================
	// [디버그]
	// ===================================================================

	// Script/Answer 테이블 QuestID 짝 검증 (데이터 입력 실수 로그 확인용)
	UFUNCTION(BlueprintCallable, Category = "Quest|Debug")
	void ValidateQuestTablesMatch();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	// ----- 런타임 상태 -----

	UPROPERTY()
	TMap<FName, EQuestState> QuestStates;

	// 수락 순서
	UPROPERTY()
	TArray<FName> QuestOrder;

	UPROPERTY()
	TMap<FName, int32> QuestHintLevels;

	// NPCId → 스토리 단계
	UPROPERTY()
	TMap<FName, int32> NPCStoryStages;

	UPROPERTY()
	FName PinnedQuestID;

	// 마지막 퀘스트 결과
	UPROPERTY()
	FCPQuestResult QuestResult;

	// ----- 내부 헬퍼 -----

	// 스크립트 테이블 조회 (없으면 nullptr, 경고 로그 없음)
	FQuestData* FindScriptData(FName QuestID) const;

	// bOutIsRandom: 어느 테이블에서 찾았는지 함께 반환
	FQuestAnswerData* FindAnswerData(FName QuestID, bool* bOutIsRandom = nullptr) const;

	// 빈 배열이면 빈 텍스트 반환
	FText PickRandomReaction(const TArray<FText>& Reactions) const;

};