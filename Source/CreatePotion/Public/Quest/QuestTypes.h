// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.generated.h"

// ===== DT 1: 텍스트 스크립트 (마을 퀘스트/저널용) =====
USTRUCT(BlueprintType)
struct FQuestData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	FText QuestText_Full;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	FText QuestText_Summary;
};

// ===== DT 2: 조건/정답 데이터 (포션 메이킹 힌트/답지) =====

// 퀘스트가 요구하는 효능
USTRUCT(BlueprintType)
struct FQuestEffectRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag Axis;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-3", ClampMax = "3"))
	int32 MinValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-3", ClampMax = "3"))
	int32 MaxValue = 3;
};

USTRUCT(BlueprintType)
struct FQuestAnswerData : public FTableRowBase
{
	GENERATED_BODY()

	// 공방 세션 진입 시 처음에 보여줄 서술형 힌트 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	FText SessionHintText;

	// 2차 힌트 - "네? 그게 뭐죠?" 버튼 눌렀을 때 보여줌 (구체적)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	FText SessionHintText_Detailed;

	// 실제 판정 기준 - UI가 절대 직접 접근하면 안 됨
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hidden")
	TArray<FQuestEffectRequirement> RequestedEffects;
};

// ===== 상태 관리 =====
UENUM(BlueprintType)
enum class EQuestState : uint8
{
	NotAccepted,
	Accepted,
	Completed
};

// 조건 충족 결과 (납품 시 항목별 피드백용)
UENUM(BlueprintType)
enum class EConditionMatchResult : uint8
{
	Correct,
	TooHigh,
	TooLow,
	WrongTag
};