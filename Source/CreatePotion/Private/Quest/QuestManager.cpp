// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/Quest/QuestManager.h"
#include "Settings/CPDTSettings.h"

// ===================================================================
// [초기화 - GameInstanceSubsystem 생성 시 자동 호출]
// ===================================================================

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (const UCPDTSettings* Settings = GetDefault<UCPDTSettings>())
	{
		QuestScriptTable = Settings->QuestScriptTable.LoadSynchronous();
		QuestAnswerTable = Settings->QuestAnswerTable.LoadSynchronous();
		RandomQuestAnswerTable = Settings->RandomQuestAnswerTable.LoadSynchronous();

		auto LogTableLoad = [](const TCHAR* Name, const UDataTable* Table)
			{
				if (Table)
				{
					UE_LOG(LogTemp, Log, TEXT("%s 로드 성공"), Name);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s 로드 실패 - CPDTSettings 확인 필요"), Name);
				}
			};
		LogTableLoad(TEXT("QuestScriptTable"), QuestScriptTable.Get());
		LogTableLoad(TEXT("QuestAnswerTable"), QuestAnswerTable.Get());
		LogTableLoad(TEXT("RandomQuestAnswerTable"), RandomQuestAnswerTable.Get());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UCPDTSettings를 찾을 수 없습니다."));
	}
}

// ===================================================================
// [상태 관리]
// ===================================================================

void UQuestManager::AcceptQuest(FName QuestID)
{
	if (GetQuestState(QuestID) == EQuestState::Completed)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 완료된 퀘스트입니다: %s"), *QuestID.ToString());
		return;
	}

	// 처음 수락하는 퀘스트라면 순서 기록에 추가
	if (!QuestStates.Contains(QuestID))
	{
		QuestOrder.Add(QuestID);
	}

	QuestStates.Add(QuestID, EQuestState::Accepted);
	OnQuestUpdated.Broadcast(QuestID, EQuestState::Accepted);
	UE_LOG(LogTemp, Log, TEXT("퀘스트 수락: %s"), *QuestID.ToString());
}

// 한 번도 언급되지 않은 QuestID는 NotAccepted 반환
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

	// 스토리 단계 진행 (방송보다 먼저)
	if (const FQuestData* Quest = FindScriptData(QuestID))
	{
		if (Quest->AdvanceToStage > GetNPCStoryStage(Quest->OwningNPCId))
		{
			NPCStoryStages.Add(Quest->OwningNPCId, Quest->AdvanceToStage);
			UE_LOG(LogTemp, Log, TEXT("NPC [%s] 스토리 단계 %d(으)로 진행"),
				*Quest->OwningNPCId.ToString(), Quest->AdvanceToStage);
		}
	}

	OnQuestUpdated.Broadcast(QuestID, EQuestState::Completed);
	OnRewardGranted.Broadcast(GetRewardGold(QuestID));

	UE_LOG(LogTemp, Log, TEXT("퀘스트 [%s] 완료 처리"), *QuestID.ToString());
}

bool UQuestManager::IsRandomQuest(FName QuestID) const
{
	bool bIsRandom = false;
	FindAnswerData(QuestID, &bIsRandom);
	return bIsRandom;
}

// 랜덤 퀘스트 하나를 무작위로 뽑아 반환 (모브 NPC 등장 시 사용)
FName UQuestManager::GetRandomQuestID() const
{
	if (!RandomQuestAnswerTable) return NAME_None;

	TArray<FName> AllIDs = RandomQuestAnswerTable->GetRowNames();

	// 이미 수락했거나 완료한 퀘스트는 후보에서 제외
	AllIDs.RemoveAll([this](const FName& ID) {
		return GetQuestState(ID) != EQuestState::NotAccepted;
		});

	if (AllIDs.Num() == 0) return NAME_None;

	int32 RandomIndex = FMath::RandRange(0, AllIDs.Num() - 1);
	return AllIDs[RandomIndex];
}

// ===================================================================
// [저널 - 추적 목록]
// ===================================================================

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

// ===================================================================
// [목표 UI / 마커]
// ===================================================================

FName UQuestManager::GetActiveGoalQuestID() const
{
	// 고정된 퀘스트가 있고 아직 진행 중이면 표시
	if (!PinnedQuestID.IsNone() && GetQuestState(PinnedQuestID) == EQuestState::Accepted)
	{
		return PinnedQuestID;
	}
	return NAME_None;
}

void UQuestManager::SetPinnedQuest(FName QuestID)
{
	PinnedQuestID = QuestID;
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

// ===================================================================
// [텍스트 조회 - UI 전용]
// 텍스트만 반환하며 판정 데이터(RequestedEffects)에는 접근하지 않음
// ===================================================================

FText UQuestManager::GetQuestTitle(FName QuestID) const
{
	const FQuestData* Quest = FindScriptData(QuestID);
	return Quest ? Quest->QuestTitle : FText::GetEmpty();
}

TArray<FText> UQuestManager::GetQuestScriptLines(FName QuestID) const
{
	const FQuestData* Quest = FindScriptData(QuestID);
	return Quest ? Quest->QuestScriptLines : TArray<FText>();
}

FText UQuestManager::GetQuestScriptTextJoined(FName QuestID) const
{
	return FText::Join(FText::FromString(TEXT("\n")), GetQuestScriptLines(QuestID));
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

// ===================================================================
// [스토리 진행 - NPC별 독립 진행도]
// ===================================================================

// 기록 없으면 0단계로 취급
int32 UQuestManager::GetNPCStoryStage(FName NPCId) const
{
	const int32* Stage = NPCStoryStages.Find(NPCId);
	return Stage ? *Stage : 0;
}

// 소속 NPC의 현재 스토리 단계가 요구 단계 이상인지 확인
bool UQuestManager::IsQuestStoryUnlocked(FName QuestID) const
{
	const FQuestData* Quest = FindScriptData(QuestID);
	if (!Quest) return true;

	return Quest->RequiredStoryStage <= GetNPCStoryStage(Quest->OwningNPCId);
}

TArray<FText> UQuestManager::GetNPCStoryLines(FName QuestID) const
{
	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->NPCStoryLines : TArray<FText>();
}

int32 UQuestManager::GetQuestHintLevel(FName QuestID) const
{
	const int32* Level = QuestHintLevels.Find(QuestID);
	return Level ? *Level : 0;
}

void UQuestManager::SetQuestHintLevel(FName QuestID, int32 NewLevel)
{
	QuestHintLevels.Add(QuestID, NewLevel);
	UE_LOG(LogTemp, Log, TEXT("퀘스트 %s 힌트 단계 %d(으)로 갱신"), *QuestID.ToString(), NewLevel);
}

// ===================================================================
// [납품 판정 - 퍼즐 시스템 연결 지점]
// ===================================================================

// - PotionResult: 퍼즐 시스템이 만든 최종 태그 배열 (제작 과정은 알 필요 없음)
// - Perfect인 경우 CompleteQuest()를 호출해 완료 처리 (보상 방송은 CompleteQuest에서 수행)
EDeliveryGrade UQuestManager::TryDeliver(FName QuestID, const TArray<FGameplayTag>& PotionResult)
{
	if (GetQuestState(QuestID) == EQuestState::Completed)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 완료 처리된 퀘스트입니다: %s"), *QuestID.ToString());
		return EDeliveryGrade::Fail;
	}

	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	if (!Answer) return EDeliveryGrade::Fail;

	// 요청된 태그가 모두 포함되어야 완료. 하나라도 빠지면 실패.
	bool bAllTagsMatched = true;
	for (const FQuestEffectRequirement& Req : Answer->RequestedEffects)
	{
		if (!PotionResult.Contains(Req.Axis))
		{
			bAllTagsMatched = false;
			break;
		}
	}

	const EDeliveryGrade Grade = bAllTagsMatched ? EDeliveryGrade::Perfect : EDeliveryGrade::Fail;

	if (Grade == EDeliveryGrade::Perfect)
	{
		CompleteQuest(QuestID);
	}

	UE_LOG(LogTemp, Log, TEXT("퀘스트 %s 납품 결과: %s"), *QuestID.ToString(), bAllTagsMatched ? TEXT("전체 충족") : TEXT("일부 누락"));

	return Grade;
}

// 조건별 세부 판정
// - 1단계: 정답이 요구한 조건을 포션이 각각 만족했는지 확인
// - 2단계: 포션에 섞인 태그 중 정답이 요구하지 않은 여분 태그 확인
TArray<FConditionEvaluation> UQuestManager::EvaluateConditions(FName QuestID, const TArray<FGameplayTag>& PotionResult) const
{
	TArray<FConditionEvaluation> Results;

	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	if (!Answer) return Results;

	// 1. 정답 기준 순회
	for (const FQuestEffectRequirement& Req : Answer->RequestedEffects)
	{
		FConditionEvaluation Eval;
		Eval.Axis = Req.Axis;
		Eval.bWasRequested = true;
		Eval.Result = PotionResult.Contains(Req.Axis) ? EConditionMatchResult::Correct : EConditionMatchResult::WrongTag;
		Results.Add(Eval);
	}

	// 2. 포션 기준 순회 - 여분 태그 탐지
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

// 판정 결과에 맞는 NPC 반응 대사 (랜덤)
// - 여분 태그: OnWrongTagReactions
// - 요청 태그 충족: 해당 축의 OnMatchReactions
// - 요청 태그 누락: 해당 축의 OnMissingReactions
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

// ===================================================================
// [결과]
// ===================================================================

FCPQuestResult UQuestManager::CreateResult(FName QuestID, const TArray<FGameplayTag>& PotionResult)
{
	if (QuestID.IsNone()) return FCPQuestResult{};
	if (!QuestResult.QuestId.IsNone())
	{
		ClearResult();
	}

	QuestResult.QuestId = QuestID;
	QuestResult.RewardAmount = GetRewardGold(QuestID);
	QuestResult.TipAmount = 0;

	if (GetQuestCompletionType(QuestID) == EQuestCompletionType::Potion)
	{
		// 포션형은 여기서 판정·완료까지 처리
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

// ===================================================================
// [완료 방식별 트리거] - 모두 Accepted 상태에서만 동작
// ===================================================================

EQuestCompletionType UQuestManager::GetQuestCompletionType(FName QuestID) const
{
	const FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->CompletionType : EQuestCompletionType::Potion;
}

bool UQuestManager::CompleteQuestByDialogue(FName QuestID)
{
	if (GetQuestState(QuestID) != EQuestState::Accepted) return false;

	if (GetQuestCompletionType(QuestID) != EQuestCompletionType::Dialogue)
	{
		UE_LOG(LogTemp, Warning, TEXT("퀘스트 %s 는 대화 완료 방식이 아닙니다."), *QuestID.ToString());
		return false;
	}

	CompleteQuest(QuestID);
	CreateResult(QuestID);
	return true;
}

bool UQuestManager::TryCompleteQuestByFindTarget(FName QuestID, FName FoundTargetId)
{
	if (GetQuestState(QuestID) != EQuestState::Accepted) return false;

	if (GetQuestCompletionType(QuestID) != EQuestCompletionType::FindTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("퀘스트 %s 는 대상 찾기 완료 방식이 아닙니다."), *QuestID.ToString());
		return false;
	}

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

// ===================================================================
// [디버그 - 테이블 무결성 검증]
// Script/Answer 테이블은 같은 QuestID(Row Name)로 짝이 맞아야 함
// ===================================================================

void UQuestManager::ValidateQuestTablesMatch()
{
	if (!QuestScriptTable || !QuestAnswerTable) return;

	TArray<FName> ScriptIDs = QuestScriptTable->GetRowNames();
	TArray<FName> AnswerIDs = QuestAnswerTable->GetRowNames();
	if (RandomQuestAnswerTable)
	{
		AnswerIDs.Append(RandomQuestAnswerTable->GetRowNames());
	}

	for (const FName& ID : ScriptIDs)
	{
		if (!AnswerIDs.Contains(ID))
		{
			UE_LOG(LogTemp, Error, TEXT("퀘스트 %s 의 정답 데이터가 없습니다!"), *ID.ToString());
		}
	}

	for (const FName& ID : AnswerIDs)
	{
		if (!ScriptIDs.Contains(ID))
		{
			UE_LOG(LogTemp, Error, TEXT("퀘스트 %s 의 텍스트 데이터(QuestScriptTable)가 없습니다!"), *ID.ToString());
		}
	}

	// OwningNPCId 없이 스토리 단계 값만 설정된 경우 경고
	for (const FName& ID : ScriptIDs)
	{
		const FQuestData* Quest = FindScriptData(ID);
		if (!Quest) continue;

		if (Quest->OwningNPCId.IsNone() && (Quest->RequiredStoryStage > 0 || Quest->AdvanceToStage >= 0))
		{
			UE_LOG(LogTemp, Warning, TEXT("퀘스트 %s 는 OwningNPCId가 비어있는데 스토리 단계 값이 설정되어 있습니다."), *ID.ToString());
		}
	}
}

// ===================================================================
// [내부 헬퍼]
// ===================================================================

FQuestData* UQuestManager::FindScriptData(FName QuestID) const
{
	return QuestScriptTable ? QuestScriptTable->FindRow<FQuestData>(QuestID, TEXT(""), false) : nullptr;
}

// 고유 테이블 → 랜덤 테이블 순으로 조회
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