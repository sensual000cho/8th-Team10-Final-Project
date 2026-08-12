#pragma once

#include "Public/Data/CPForageableItemData.h" // FAlchemyProperty 참조용 (채집물/연금 태그 구조체)
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuestTypes.h"
#include "QuestManager.generated.h"

// 퀘스트 상태가 바뀔 때마다(수락/완료 등) 방송되는 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestUpdated, FName, QuestID, EQuestState, NewState);

// 포션 납품 시 판정되는 등급.
// TryDeliver()의 반환값으로 사용되며, 완성된 포션이 퀘스트 조건을
// 몇 개나 만족했는지에 따라 결정됨.
UENUM(BlueprintType)
enum class EDeliveryGrade : uint8
{
	Fail,     // 조건을 하나도 만족 못함
	Okay,     // 일부 조건만 만족
	Good,     // 대부분(전체-1개 이상) 조건 만족
	Perfect   // 모든 조건 만족
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

	// 퀘스트 상태 변경 알림 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestUpdated OnQuestUpdated;

	// 플레이어가 퀘스트를 수락한 순서대로 QuestID를 저장하는 배열
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<FName> AcceptedQuestOrder;
	
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

	// ===================================================================
	// [텍스트 조회 - UI 전용 참조 함수]
	// ===================================================================

	// 저널 목록용 - 짧은 제목
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FText GetQuestTitle(FName QuestID) const;

	// [텍스트-원문] 마을 NPC가 퀘스트를 제안할 때 하는 원문 대사
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FText GetQuestFullText(FName QuestID) const;

	// [텍스트-요약] 수락 후 퀘스트 저널에서 다시 확인할 때 보여줄 요약
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FText GetQuestSummaryText(FName QuestID) const;

	// ===================================================================
	// [세션 힌트 - 단계별 조회 및 현재 단계 자동 관리]
	// GetSessionHintText/Detailed/Detailed2 : 단계별 개별 조회 (저수준)
	// GetCurrentSessionHintText : 저장된 현재 단계에 맞는 힌트를 자동으로 골라 반환 (UI 권장 사용)
	// ===================================================================

	// [세션힌트 1차]
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FText GetSessionHintText(FName QuestID) const;

	// [세션힌트 2차]
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FText GetSessionHintTextDetailed(FName QuestID) const;

	// [세션힌트 3차]
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FText GetSessionHintTextDetailed2(FName QuestID) const;

	// 특정 퀘스트의 현재 힌트 단계 조회
	UFUNCTION(BlueprintCallable, Category = "Quest")
	int32 GetQuestHintLevel(FName QuestID) const;

	// 특정 퀘스트의 힌트 단계 갱신 (저장)
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetQuestHintLevel(FName QuestID, int32 NewLevel);

	// 현재 저장된 힌트 단계에 맞는 텍스트 자동 반환 (UI는 이 함수만 호출하면 됨)
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FText GetCurrentSessionHintText(FName QuestID) const;

	// ===================================================================
	// [납품 판정 - 퍼즐 시스템과의 연결 지점]
	// ===================================================================

	// 퀘스트가 요구하는 조건과 비교, 등급을 매기고 만족 시 상태를 Completed로 전환
	UFUNCTION(BlueprintCallable, Category = "Quest")
	EDeliveryGrade TryDeliver(FName QuestID, const TArray<FAlchemyProperty>& PotionResult);

	// 조건 하나하나에 대한 세부 판정 (O/Up/Down/태그오답)
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FConditionEvaluation> EvaluateConditions(FName QuestID, const TArray<FAlchemyProperty>& PotionResult) const;

	// 요구 조건 조회
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FQuestEffectRequirement> GetQuestEffectRequirements(FName QuestId) const;
	
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

protected:
	// Subsystem 생성 시 자동 호출됨. QuestScriptTable/QuestAnswerTable을 자동으로 찾아 연결함.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};