#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuestTypes.h"
#include "QuestManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestUpdated, FName, QuestID, EQuestState, NewState);
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
	FCPQuestSaveData GetSaveData();
	void LoadSaveData(const FCPQuestSaveData& SaveData);
	void ResetData();

	UPROPERTY()
	TObjectPtr<UDataTable> QuestScriptTable;

	UPROPERTY()
	TObjectPtr<UDataTable> QuestAnswerTable;

	UPROPERTY()
	TObjectPtr<UDataTable> RandomQuestAnswerTable;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestUpdated OnQuestUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnRewardGranted OnRewardGranted;

	UFUNCTION(BlueprintCallable, Category = "Quest|State")
	void AcceptQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest|State")
	void CompleteQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest|State")
	EQuestState GetQuestState(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool IsRandomQuest(FName QuestID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quest|State")
	TArray<FName> GetRandomQuestIDs(int32 MaxCount = 6) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Journal")
	TArray<FName> GetAllTrackedQuestIDs() const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Journal")
	TArray<FName> GetTrackedTownQuestIDs() const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Journal")
	TArray<FName> GetTrackedRandomQuestIDs() const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Goal")
	FName GetActiveGoalQuestID() const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Goal")
	void SetPinnedQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest|Goal")
	FName GetPinnedQuest() const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Goal")
	EQuestMarkerState GetMarkerStateForQuests(const TArray<FName>& QuestIDs, const TMap<FName, int32>& HeldItemCounts) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Text")
	FText GetQuestTitle(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Text")
	TArray<FCPDialogueLine> GetQuestScriptLines(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Text")
	FText GetQuestScriptTextJoined(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Text")
	FText GetQuestSummaryText(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Text")
	FText GetCodexSummaryText(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Text")
	FText GetSessionHintText(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Story")
	int32 GetNPCStoryStage(FName NPCId) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Story")
	bool IsQuestStoryUnlocked(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Story")
	TArray<FCPDialogueLine> GetNPCStoryLines(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Story")
	int32 GetQuestHintLevel(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Story")
	void SetQuestHintLevel(FName QuestID, int32 NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Quest|Delivery")
	EDeliveryGrade TryDeliver(FName QuestID, const TArray<FGameplayTag>& PotionResult);

	UFUNCTION(BlueprintCallable, Category = "Quest|Delivery")
	TArray<FConditionEvaluation> EvaluateConditions(FName QuestID, const TArray<FGameplayTag>& PotionResult) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Delivery")
	FText GetReactionText(FName QuestID, const FConditionEvaluation& Evaluation) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Delivery")
	TArray<FQuestEffectRequirement> GetQuestEffectRequirements(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Delivery")
	int32 GetRewardGold(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	bool ShouldConsumeRequiredItems(FName QuestID) const;

	FCPQuestResult CreateResult(FName QuestID, const TArray<FGameplayTag>& PotionResult = TArray<FGameplayTag>());

	UFUNCTION(BlueprintCallable, Category = "Quest|Result")
	FCPQuestResult GetResult(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Result")
	bool ClearResult();

	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	EQuestCompletionType GetQuestCompletionType(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	bool CompleteQuestByDialogue(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	bool TryCompleteQuestByFindTarget(FName QuestID, FName FoundTargetId);

	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	FName GetTargetIdentifier(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	bool TryCompleteQuestByItem(FName QuestID, const TMap<FName, int32>& HeldItemCounts);

	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	bool IsItemQuestCompletable(FName QuestID, const TMap<FName, int32>& HeldItemCounts) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	void GetRequiredItemInfo(FName QuestID, TArray<FName>& OutItemIDs, int32& OutRequiredCount) const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Completion")
	FText GetItemProgressText(FName QuestID, const TMap<FName, int32>& HeldItemCounts) const;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	UPROPERTY()
	TMap<FName, EQuestState> QuestStates;

	UPROPERTY()
	TArray<FName> QuestOrder;

	UPROPERTY()
	TMap<FName, int32> QuestHintLevels;

	UPROPERTY()
	TMap<FName, int32> NPCStoryStages;

	UPROPERTY()
	FName PinnedQuestID;

	UPROPERTY()
	bool bHasEverPinned = false;

	UPROPERTY()
	FCPQuestResult QuestResult;

	FQuestData* FindScriptData(FName QuestID) const;
	FQuestAnswerData* FindAnswerData(FName QuestID, bool* bOutIsRandom = nullptr) const;
	FText PickRandomReaction(const TArray<FText>& Reactions) const;
};