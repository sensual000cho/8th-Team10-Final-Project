#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Sound/SoundBase.h"
#include "QuestTypes.generated.h"

USTRUCT(BlueprintType)
struct FCPDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TSoftObjectPtr<USoundBase> Voice;
};

USTRUCT(BlueprintType)
struct FQuestData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	FText QuestTitle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing")
	TArray<FCPDialogueLine> QuestScriptLines;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerFacing", meta = (MultiLine = true))
	FText QuestSummaryText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Codex")
	FText CodexSummaryText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	FName OwningNPCId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	int32 RequiredStoryStage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	int32 AdvanceToStage = -1;
};

UENUM(BlueprintType)
enum class EQuestCompletionType : uint8
{
	Dialogue,
	FindTarget,
	ItemCollection,
	Potion,
	FieldGimmick
};

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
	TArray<FCPDialogueLine> NPCStoryLines;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Judgement")
	TArray<FQuestEffectRequirement> RequestedEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reaction")
	TArray<FText> OnWrongTagReactions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	int32 RewardGold = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Completion")
	EQuestCompletionType CompletionType = EQuestCompletionType::Potion;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Completion|FindTarget",
		meta = (EditCondition = "CompletionType == EQuestCompletionType::FindTarget", EditConditionHides))
	FName TargetIdentifier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Completion|ItemCollection",
		meta = (EditCondition = "CompletionType == EQuestCompletionType::ItemCollection", EditConditionHides))
	TArray<FName> RequiredItemIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Completion|ItemCollection",
		meta = (EditCondition = "CompletionType == EQuestCompletionType::ItemCollection", EditConditionHides))
	int32 RequiredItemCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Completion|ItemCollection",
		meta = (EditCondition = "CompletionType == EQuestCompletionType::ItemCollection", EditConditionHides))
	TArray<FText> RequiredItemHintNames;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Completion|ItemCollection",
		meta = (EditCondition = "CompletionType == EQuestCompletionType::ItemCollection", EditConditionHides))
	bool bConsumeRequiredItems = true;
};

UENUM(BlueprintType)
enum class EQuestState : uint8
{
	NotAccepted,
	Accepted,
	Completed
};

UENUM(BlueprintType)
enum class EQuestMarkerState : uint8
{
	None,
	Available,
	InProgress,
	Completable
};

UENUM(BlueprintType)
enum class EConditionMatchResult : uint8
{
	Correct,
	WrongTag
};

USTRUCT(BlueprintType)
struct FConditionEvaluation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Quest|Delivery")
	FGameplayTag Axis;

	UPROPERTY(BlueprintReadOnly, Category = "Quest|Delivery")
	EConditionMatchResult Result = EConditionMatchResult::Correct;

	UPROPERTY(BlueprintReadOnly, Category = "Quest|Delivery")
	bool bWasRequested = true;
};

UENUM(BlueprintType)
enum class EDeliveryGrade : uint8
{
	Fail,
	Perfect
};

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