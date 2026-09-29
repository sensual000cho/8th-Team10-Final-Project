#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.generated.h"

// ===================================================================
// [DT 1: 텍스트 스크립트 (마을 퀘스트/저널용)]
// 플레이어에게 보이는 텍스트만 담음. 판정용 데이터는 포함하지 않음.
// ===================================================================

USTRUCT(BlueprintType)
struct FQuestData : public FTableRowBase
{
	GENERATED_BODY()

	// 저널 목록용 짧은 제목
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	FText QuestTitle;

	// NPC 제안 원문 대사
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	TArray<FText> QuestScriptLines;

	// 수락 후 저널 요약
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	FText QuestSummaryText;

	// 인물 도감 회고 요약 (랜덤 퀘스트는 미사용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Codex")
	FText CodexSummaryText;

	// ----- 스토리 진행 (NPC별 독립 관리) -----

	// 소속 NPC 식별자 (UCPNPCDataAsset::NPCName과 일치해야 함)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	FName OwningNPCId;

	// 노출에 필요한 최소 NPC 스토리 단계
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	int32 RequiredStoryStage = 0;

	// 완료 시 진행시킬 NPC 스토리 단계 (-1: 영향 없음)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	int32 AdvanceToStage = -1;
};

// ===================================================================
// [DT 2: 조건/정답 데이터]
// 판정 조건, 세션 힌트, 완료 방식별 데이터를 담음.
// ===================================================================

// 퀘스트 완료 방식
UENUM(BlueprintType)
enum class EQuestCompletionType : uint8
{
	Dialogue,        // 대화로 완료 (CompleteQuestByDialogue)
	FindTarget,      // 필드 대상 발견 (TryCompleteQuestByFindTarget)
	ItemCollection,  // 아이템 제출 (TryCompleteQuestByItem)
	Potion,          // 포션 납품/투척 (TryDeliver)
	FieldGimmick     // 필드 기믹
};

// 퀘스트가 요구하는 효능 조건 하나
USTRUCT(BlueprintType)
struct FQuestEffectRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	FGameplayTag Axis;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reaction")
	TArray<FText> OnMatchReactions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reaction")
	TArray<FText> OnMissingReactions;
};

USTRUCT(BlueprintType)
struct FQuestAnswerData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	FText SessionHintText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	TArray<FText> NPCStoryLines;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Judgement")
	TArray<FQuestEffectRequirement> RequestedEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reaction")
	TArray<FText> OnWrongTagReactions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	int32 RewardGold = 100;

	// 완료 방식
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Completion")
	EQuestCompletionType CompletionType = EQuestCompletionType::Potion;

	// [FindTarget] 찾아야 하는 대상 식별자
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Completion|FindTarget",
		meta = (EditCondition = "CompletionType == EQuestCompletionType::FindTarget", EditConditionHides))
	FName TargetIdentifier;

	// [ItemCollection] 제출할 아이템 ID 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Completion|ItemCollection",
		meta = (EditCondition = "CompletionType == EQuestCompletionType::ItemCollection", EditConditionHides))
	TArray<FName> RequiredItemIDs;

	// [ItemCollection] 아이템당 필요 수량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Completion|ItemCollection",
		meta = (EditCondition = "CompletionType == EQuestCompletionType::ItemCollection", EditConditionHides))
	int32 RequiredItemCount = 1;

	// [ItemCollection] UI 표시용 모호한 이름 (RequiredItemIDs와 순서 일치)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Completion|ItemCollection",
		meta = (EditCondition = "CompletionType == EQuestCompletionType::ItemCollection", EditConditionHides))
	TArray<FText> RequiredItemHintNames;
};

// ===================================================================
// [상태 관리 및 판정 결과]
// ===================================================================

// 퀘스트 진행 상태
UENUM(BlueprintType)
enum class EQuestState : uint8
{
	NotAccepted,
	Accepted,
	Completed
};

// NPC 머리 위 마커 상태
UENUM(BlueprintType)
enum class EQuestMarkerState : uint8
{
	None,          // 표시 없음
	Available,     // 새 퀘스트 있음
	InProgress,    // 진행 중, 완료 조건 미충족
	Completable    // 완료 조건 충족
};

// 조건별 판정 결과
UENUM(BlueprintType)
enum class EConditionMatchResult : uint8
{
	Correct,   // 조건 정확히 충족
	WrongTag   // 요청하지 않은 축이거나 아예 없음
};

// 조건 판정 결과 하나 (축 + 판정)
USTRUCT(BlueprintType)
struct FConditionEvaluation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Quest|Delivery")
	FGameplayTag Axis;

	UPROPERTY(BlueprintReadOnly, Category = "Quest|Delivery")
	EConditionMatchResult Result = EConditionMatchResult::Correct;

	// true: 정답이 요구한 조건 / false: 포션에 여분으로 섞인 태그
	UPROPERTY(BlueprintReadOnly, Category = "Quest|Delivery")
	bool bWasRequested = true;
};

// 포션 납품 판정 등급 (TryDeliver() 반환값)
UENUM(BlueprintType)
enum class EDeliveryGrade : uint8
{
	Fail,     // 요구 태그 중 하나라도 누락
	Perfect   // 요구 태그 모두 충족
};

// 퀘스트 완료 결과 (결과 UI 표시용)
USTRUCT(BlueprintType)
struct FCPQuestResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Quest|Result")
	FName QuestId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Quest|Result")
	EDeliveryGrade DeliveryGrade = EDeliveryGrade::Fail;

	UPROPERTY(BlueprintReadOnly, Category = "Quest|Result")
	TArray<FGameplayTag> CurrentEffects;

	UPROPERTY(BlueprintReadOnly, Category = "Quest|Result")
	int32 RewardAmount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Quest|Result")
	int32 TipAmount = 0;
};