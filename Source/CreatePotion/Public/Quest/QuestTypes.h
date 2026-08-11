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
	FText QuestText_Full;

	// 퀘스트 요약 (수락 후 저널에서 다시 확인할 때 표시)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	FText QuestText_Summary;
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

	// 어떤 축(효능 종류)에 대한 조건인지 (예: Alchemy.Drowsiness)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag Axis;

	// 이 값 이상이어야 함
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-3", ClampMax = "3"))
	int32 MinValue = 0;

	// 이 값 이하여야 함
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-3", ClampMax = "3"))
	int32 MaxValue = 3;
};

USTRUCT(BlueprintType)
struct FQuestAnswerData : public FTableRowBase
{
	GENERATED_BODY()

	// 1차 힌트 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	FText SessionHintText;

	// 2차 힌트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	FText SessionHintText_Detailed;

	// 3차(최종) 힌트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	FText SessionHintText_Detailed2;

	// 실제 판정 기준 - UI에선 접근 X
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hidden")
	TArray<FQuestEffectRequirement> RequestedEffects;
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

// 조건 충족 결과 (납품 시 항목별 피드백용 - O/Up/Down/태그오답)
UENUM(BlueprintType)
enum class EConditionMatchResult : uint8
{
	Correct,   // O - 조건 정확히 충족
	TooHigh,   // Up - 요구치보다 높음
	TooLow,    // Down - 요구치보다 낮음
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