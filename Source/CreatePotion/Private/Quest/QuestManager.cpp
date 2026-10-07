#include "Public/Quest/QuestManager.h"
#include "Settings/CPDTSettings.h"

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (const UCPDTSettings* Settings = GetDefault<UCPDTSettings>())
	{
		QuestScriptTable = Settings->QuestScriptTable.LoadSynchronous();
		QuestAnswerTable = Settings->QuestAnswerTable.LoadSynchronous();
		RandomQuestAnswerTable = Settings->RandomQuestAnswerTable.LoadSynchronous();
	}
}

void UQuestManager::AcceptQuest(FName QuestID)
{
	if (GetQuestState(QuestID) == EQuestState::Completed) return;

	if (!QuestStates.Contains(QuestID))
	{
		QuestOrder.Add(QuestID);
	}

	QuestStates.Add(QuestID, EQuestState::Accepted);
	OnQuestUpdated.Broadcast(QuestID, EQuestState::Accepted);
}

EQuestState UQuestManager::GetQuestState(FName QuestID) const
{
	const EQuestState* State = QuestStates.Find(QuestID);
	return State ? *State : EQuestState::NotAccepted;
}

void UQuestManager::CompleteQuest(FName QuestID)
{
	if (QuestID.IsNone()) return;
	if (GetQuestState(QuestID) == EQuestState::Completed) return;

	QuestStates.Add(QuestID, EQuestState::Completed);

	if (const FQuestData* Quest = FindScriptData(QuestID))
	{
		if (Quest->AdvanceToStage > GetNPCStoryStage(Quest->OwningNPCId))
		{
			NPCStoryStages.Add(Quest->OwningNPCId, Quest->AdvanceToStage);
		}
	}

	if (PinnedQuestID == QuestID)
	{
		PinnedQuestID = NAME_None;
	}

	OnQuestUpdated.Broadcast(QuestID, EQuestState::Completed);
	OnRewardGranted.Broadcast(GetRewardGold(QuestID));
}

bool UQuestManager::IsRandomQuest(FName QuestID) const
{
	bool bIsRandom = false;
	FindAnswerData(QuestID, &bIsRandom);
	return bIsRandom;
}

FName UQuestManager::GetRandomQuestID() const
{
	if (!RandomQuestAnswerTable) return NAME_None;

	TArray<FName> AllIDs = RandomQuestAnswerTable->GetRowNames();
	AllIDs.RemoveAll([this](const FName& ID) {
		return GetQuestState(ID) != EQuestState::NotAccepted;
		});

	if (AllIDs.Num() == 0) return NAME_None;

	return AllIDs[FMath::RandRange(0, AllIDs.Num() - 1)];
}

TArray<FName> UQuestManager::GetAllTrackedQuestIDs() const
{
	return QuestOrder;
}

TArray<FName> UQuestManager::GetTrackedTownQuestIDs() const
{
	TArray<FName> Result;
	for (const FName& QuestID : QuestOrder)
	{
		if (!IsRandomQuest(QuestID))
		{
			Result.Add(QuestID);
		}
	}
	return Result;
}

TArray<FName> UQuestManager::GetTrackedRandomQuestIDs() const
{
	TArray<FName> Result;
	for (const FName& QuestID : QuestOrder)
	{
		if (IsRandomQuest(QuestID))
		{
			Result.Add(QuestID);
		}
	}
	return Result;
}

FName UQuestManager::GetActiveGoalQuestID() const
{
	if (!PinnedQuestID.IsNone() && GetQuestState(PinnedQuestID) == EQuestState::Accepted)
	{
		return PinnedQuestID;
	}

	if (!bHasEverPinned)
	{
		for (const FName& QuestID : QuestOrder)
		{
			if (GetQuestState(QuestID) == EQuestState::Accepted)
			{
				return QuestID;
			}
		}
	}

	return NAME_None;
}

void UQuestManager::SetPinnedQuest(FName QuestID)
{
	PinnedQuestID = QuestID;
	bHasEverPinned = true;
	OnQuestUpdated.Broadcast(QuestID, GetQuestState(QuestID));
}

FName UQuestManager::GetPinnedQuest() const
{
	return PinnedQuestID;
}

EQuestMarkerState UQuestManager::GetMarkerStateForQuests(const TArray<FName>& QuestIDs, const TMap<FName, int32>& HeldItemCounts) const
{
	bool bHasInProgress = false;

	for (const FName& QuestID : QuestIDs)
	{
		if (QuestID.IsNone()) continue;

		EQuestState State = GetQuestState(QuestID);

		if (State == EQuestState::NotAccepted && IsQuestStoryUnlocked(QuestID))
		{
			return EQuestMarkerState::Available;
		}

		if (State == EQuestState::Accepted)
		{
			EQuestCompletionType Type = GetQuestCompletionType(QuestID);
			bool bIsCompletable = false;

			if (Type == EQuestCompletionType::Dialogue)
			{
				bIsCompletable = true;
			}
			else if (Type == EQuestCompletionType::ItemCollection)
			{
				bIsCompletable = IsItemQuestCompletable(QuestID, HeldItemCounts);
			}

			if (bIsCompletable)
			{
				return EQuestMarkerState::Completable;
			}
			bHasInProgress = true;
		}
	}

	return bHasInProgress ? EQuestMarkerState::InProgress : EQuestMarkerState::None;
}

FText UQuestManager::GetQuestTitle(FName QuestID) const
{
	const FQuestData* Quest = FindScriptData(QuestID);
	return Quest ? Quest->QuestTitle : FText::GetEmpty();
}

TArray<FCPDialogueLine> UQuestManager::GetQuestScriptLines(FName QuestID) const
{
	const FQuestData* Quest = FindScriptData(QuestID);
	return Quest ? Quest->QuestScriptLines : TArray<FCPDialogueLine>();
}

FText UQuestManager::GetQuestScriptTextJoined(FName QuestID) const
{
	TArray<FText> Texts;
	for (const FCPDialogueLine& Line : GetQuestScriptLines(QuestID))
	{
		Texts.Add(Line.Text);
	}
	return FText::Join(FText::FromString(TEXT("\n")), Texts);
}

FText UQuestManager::GetQuestSummaryText(FName QuestID) const
{
	const FQuestData* Quest = FindScriptData(QuestID);
	return Quest ? Quest->QuestSummaryText : FText::GetEmpty();
}

FText UQuestManager::GetCodexSummaryText(FName QuestID) const
{
	const FQuestData* Quest = FindScriptData(QuestID);
	return Quest ? Quest->CodexSummaryText : FText::GetEmpty();
}

FText UQuestManager::GetSessionHintText(FName QuestID) const
{
	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->SessionHintText : FText::GetEmpty();
}

int32 UQuestManager::GetNPCStoryStage(FName NPCId) const
{
	const int32* Stage = NPCStoryStages.Find(NPCId);
	return Stage ? *Stage : 0;
}

bool UQuestManager::IsQuestStoryUnlocked(FName QuestID) const
{
	const FQuestData* Quest = FindScriptData(QuestID);
	if (!Quest) return true;

	return Quest->RequiredStoryStage <= GetNPCStoryStage(Quest->OwningNPCId);
}

TArray<FCPDialogueLine> UQuestManager::GetNPCStoryLines(FName QuestID) const
{
	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->NPCStoryLines : TArray<FCPDialogueLine>();
}

int32 UQuestManager::GetQuestHintLevel(FName QuestID) const
{
	const int32* Level = QuestHintLevels.Find(QuestID);
	return Level ? *Level : 0;
}

void UQuestManager::SetQuestHintLevel(FName QuestID, int32 NewLevel)
{
	QuestHintLevels.Add(QuestID, NewLevel);
}

EDeliveryGrade UQuestManager::TryDeliver(FName QuestID, const TArray<FGameplayTag>& PotionResult)
{
	if (GetQuestState(QuestID) == EQuestState::Completed) return EDeliveryGrade::Fail;

	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	if (!Answer) return EDeliveryGrade::Fail;

	for (const FQuestEffectRequirement& Req : Answer->RequestedEffects)
	{
		if (!PotionResult.Contains(Req.Axis))
		{
			return EDeliveryGrade::Fail;
		}
	}

	CompleteQuest(QuestID);
	return EDeliveryGrade::Perfect;
}

TArray<FConditionEvaluation> UQuestManager::EvaluateConditions(FName QuestID, const TArray<FGameplayTag>& PotionResult) const
{
	TArray<FConditionEvaluation> Results;

	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	if (!Answer) return Results;

	for (const FQuestEffectRequirement& Req : Answer->RequestedEffects)
	{
		FConditionEvaluation Eval;
		Eval.Axis = Req.Axis;
		Eval.bWasRequested = true;
		Eval.Result = PotionResult.Contains(Req.Axis) ? EConditionMatchResult::Correct : EConditionMatchResult::WrongTag;
		Results.Add(Eval);
	}

	for (const FGameplayTag& EffectTag : PotionResult)
	{
		const bool bIsRequested = Answer->RequestedEffects.ContainsByPredicate(
			[&](const FQuestEffectRequirement& Req) { return Req.Axis == EffectTag; }
		);

		if (!bIsRequested)
		{
			FConditionEvaluation Eval;
			Eval.Axis = EffectTag;
			Eval.Result = EConditionMatchResult::WrongTag;
			Eval.bWasRequested = false;
			Results.Add(Eval);
		}
	}

	return Results;
}

FText UQuestManager::GetReactionText(FName QuestID, const FConditionEvaluation& Evaluation) const
{
	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	if (!Answer) return FText::GetEmpty();

	if (!Evaluation.bWasRequested)
	{
		return PickRandomReaction(Answer->OnWrongTagReactions);
	}

	const FQuestEffectRequirement* MatchingReq = Answer->RequestedEffects.FindByPredicate(
		[&](const FQuestEffectRequirement& Req) { return Req.Axis == Evaluation.Axis; }
	);
	if (!MatchingReq) return FText::GetEmpty();

	return Evaluation.Result == EConditionMatchResult::Correct
		? PickRandomReaction(MatchingReq->OnMatchReactions)
		: PickRandomReaction(MatchingReq->OnMissingReactions);
}

TArray<FQuestEffectRequirement> UQuestManager::GetQuestEffectRequirements(FName QuestID) const
{
	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->RequestedEffects : TArray<FQuestEffectRequirement>();
}

int32 UQuestManager::GetRewardGold(FName QuestID) const
{
	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->RewardGold : 0;
}

bool UQuestManager::ShouldConsumeRequiredItems(FName QuestID) const
{
	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->bConsumeRequiredItems : true;
}

FCPQuestResult UQuestManager::CreateResult(FName QuestID, const TArray<FGameplayTag>& PotionResult)
{
	if (QuestID.IsNone()) return FCPQuestResult{};

	ClearResult();

	QuestResult.QuestId = QuestID;
	QuestResult.RewardAmount = GetRewardGold(QuestID);
	QuestResult.TipAmount = 0;

	if (GetQuestCompletionType(QuestID) == EQuestCompletionType::Potion)
	{
		QuestResult.CurrentEffects = PotionResult;
		QuestResult.DeliveryGrade = TryDeliver(QuestID, PotionResult);
	}
	else
	{
		QuestResult.CurrentEffects.Empty();
		QuestResult.DeliveryGrade = EDeliveryGrade::Perfect;
	}

	return QuestResult;
}

FCPQuestResult UQuestManager::GetResult(FName QuestID) const
{
	return QuestResult.QuestId == QuestID ? QuestResult : FCPQuestResult{};
}

bool UQuestManager::ClearResult()
{
	if (QuestResult.QuestId.IsNone()) return false;

	QuestResult = FCPQuestResult{};
	return true;
}

EQuestCompletionType UQuestManager::GetQuestCompletionType(FName QuestID) const
{
	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->CompletionType : EQuestCompletionType::Potion;
}

bool UQuestManager::CompleteQuestByDialogue(FName QuestID)
{
	if (GetQuestState(QuestID) != EQuestState::Accepted) return false;
	if (GetQuestCompletionType(QuestID) != EQuestCompletionType::Dialogue) return false;

	CompleteQuest(QuestID);
	CreateResult(QuestID);
	return true;
}

bool UQuestManager::TryCompleteQuestByFindTarget(FName QuestID, FName FoundTargetId)
{
	if (GetQuestState(QuestID) != EQuestState::Accepted) return false;
	if (GetQuestCompletionType(QuestID) != EQuestCompletionType::FindTarget) return false;
	if (GetTargetIdentifier(QuestID) != FoundTargetId) return false;

	CompleteQuest(QuestID);
	CreateResult(QuestID);
	return true;
}

FName UQuestManager::GetTargetIdentifier(FName QuestID) const
{
	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->TargetIdentifier : NAME_None;
}

bool UQuestManager::TryCompleteQuestByItem(FName QuestID, const TMap<FName, int32>& HeldItemCounts)
{
	if (GetQuestState(QuestID) != EQuestState::Accepted) return false;
	if (!IsItemQuestCompletable(QuestID, HeldItemCounts)) return false;

	CompleteQuest(QuestID);
	CreateResult(QuestID);
	return true;
}

bool UQuestManager::IsItemQuestCompletable(FName QuestID, const TMap<FName, int32>& HeldItemCounts) const
{
	if (GetQuestCompletionType(QuestID) != EQuestCompletionType::ItemCollection) return false;

	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	if (!Answer) return false;

	for (const FName& RequiredItemID : Answer->RequiredItemIDs)
	{
		const int32* HeldCount = HeldItemCounts.Find(RequiredItemID);
		if (!HeldCount || *HeldCount < Answer->RequiredItemCount)
		{
			return false;
		}
	}
	return true;
}

void UQuestManager::GetRequiredItemInfo(FName QuestID, TArray<FName>& OutItemIDs, int32& OutRequiredCount) const
{
	OutItemIDs.Empty();
	OutRequiredCount = 0;

	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	if (!Answer) return;

	OutItemIDs = Answer->RequiredItemIDs;
	OutRequiredCount = Answer->RequiredItemCount;
}

FText UQuestManager::GetItemProgressText(FName QuestID, const TMap<FName, int32>& HeldItemCounts) const
{
	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	if (!Answer || Answer->RequiredItemIDs.Num() == 0) return FText::GetEmpty();

	TArray<FString> Lines;
	for (int32 i = 0; i < Answer->RequiredItemIDs.Num(); ++i)
	{
		const int32* HeldCount = HeldItemCounts.Find(Answer->RequiredItemIDs[i]);
		const int32 Held = HeldCount ? *HeldCount : 0;

		const FString HintName = Answer->RequiredItemHintNames.IsValidIndex(i)
			? Answer->RequiredItemHintNames[i].ToString()
			: TEXT("재료");

		Lines.Add(FString::Printf(TEXT("%s: %d / %d"), *HintName, Held, Answer->RequiredItemCount));
	}

	return FText::FromString(FString::Join(Lines, TEXT("\n")));
}

FQuestData* UQuestManager::FindScriptData(FName QuestID) const
{
	return QuestScriptTable ? QuestScriptTable->FindRow<FQuestData>(QuestID, TEXT(""), false) : nullptr;
}

FQuestAnswerData* UQuestManager::FindAnswerData(FName QuestID, bool* bOutIsRandom) const
{
	if (QuestAnswerTable)
	{
		if (FQuestAnswerData* Found = QuestAnswerTable->FindRow<FQuestAnswerData>(QuestID, TEXT(""), false))
		{
			if (bOutIsRandom) *bOutIsRandom = false;
			return Found;
		}
	}
	if (RandomQuestAnswerTable)
	{
		if (FQuestAnswerData* Found = RandomQuestAnswerTable->FindRow<FQuestAnswerData>(QuestID, TEXT(""), false))
		{
			if (bOutIsRandom) *bOutIsRandom = true;
			return Found;
		}
	}
	if (bOutIsRandom) *bOutIsRandom = false;
	return nullptr;
}

FText UQuestManager::PickRandomReaction(const TArray<FText>& Reactions) const
{
	if (Reactions.Num() == 0) return FText::GetEmpty();
	return Reactions[FMath::RandRange(0, Reactions.Num() - 1)];
}

FCPQuestSaveData UQuestManager::GetSaveData()
{
	FCPQuestSaveData SaveData;
	SaveData.QuestStates = QuestStates;
	SaveData.QuestResults = QuestOrder;
	SaveData.NPCStoryStages = NPCStoryStages;
	SaveData.PinnedQuestID = PinnedQuestID;
	return SaveData;
}

void UQuestManager::LoadSaveData(const FCPQuestSaveData& SaveData)
{
	QuestStates = SaveData.QuestStates;
	QuestOrder = SaveData.QuestResults;
	NPCStoryStages = SaveData.NPCStoryStages;
	PinnedQuestID = SaveData.PinnedQuestID;
}

void UQuestManager::ResetData()
{
	QuestStates.Reset();
	QuestOrder.Reset();
	NPCStoryStages.Reset();
	PinnedQuestID = NAME_None;
}