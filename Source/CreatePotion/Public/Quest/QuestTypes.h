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

	// 저널 목록에서 보여줄 짧은 제목
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	FText QuestTitle;

	// 퀘스트 원문 (마을 NPC가 제안할 때 하는 대사)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	TArray<FText> QuestScriptLines;

	// 퀘스트 요약 (수락 후 저널에서 다시 확인할 때 표시)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	FText QuestSummaryText;

	// ===================================================================
	// [스토리 진행 - NPC별 독립 스토리 관리용]
	// ===================================================================

	// 이 퀘스트가 소속된 NPC 식별자 (UCPNPCDataAsset::NPCName과 일치해야 함)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	FName OwningNPCId;

	// 이 퀘스트가 노출되려면 해당 NPC 스토리가 최소 몇 단계여야 하는지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	int32 RequiredStoryStage = 0;

	// 이 퀘스트를 완료하면 해당 NPC 스토리를 이 단계로 진행시킴 (-1이면 스토리 진행에 영향 없음)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	int32 AdvanceToStage = -1;

};

// ===================================================================
// [DT 2: 조건/정답 데이터 (포션 메이킹 힌트/답지)]
// 실제 판정에 쓰이는 조건과, 세션 진입 시 보여줄 힌트 텍스트를 담음.
// ===================================================================

// 퀘스트가 요구하는 효능 조건 하나 (틀 - 실제로는 배열로 여러 개 담김)
USTRUCT(BlueprintType)
struct FQuestEffectRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hidden")
	TArray<FQuestEffectRequirement> RequestedEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reaction")
	TArray<FText> OnWrongTagReactions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	int32 RewardGold = 100;
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

// 조건 충족 결과 (납품 시 항목별 피드백용 - O/태그오답)
UENUM(BlueprintType)
enum class EConditionMatchResult : uint8
{
	Correct,   // O - 조건 정확히 충족
	WrongTag   // 태그오답 - 요청하지 않은 축이거나 아예 없음
};

// 조건 판정 결과 하나 (어떤 축에 대한 결과인지 + 판정 결과)
USTRUCT(BlueprintType)
struct FConditionEvaluation
{
	GENERATED_BODY()

	// 어떤 축(태그)에 대한 결과인지
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Axis;

	// 판정 결과
	UPROPERTY(BlueprintReadOnly)
	EConditionMatchResult Result = EConditionMatchResult::Correct;

	// 이게 정답이 요구한 조건인지(true), 아니면 포션에 여분으로 섞인 태그인지(false)
	UPROPERTY(BlueprintReadOnly)
	bool bWasRequested = true;
};