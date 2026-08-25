// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/Quest/QuestManager.h"
#include "Quest/QuestSettings.h"

// ===================================================================
// [초기화 - GameInstanceSubsystem 생성 시 자동 호출]
// ===================================================================

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (const UQuestSettings* Settings = GetDefault<UQuestSettings>())
	{
		QuestScriptTable = Settings->QuestScriptTable.LoadSynchronous();
		QuestAnswerTable = Settings->QuestAnswerTable.LoadSynchronous();
		RandomQuestAnswerTable = Settings->RandomQuestAnswerTable.LoadSynchronous();

		UE_LOG(LogTemp, Warning, TEXT("QuestScriptTable 로드 %s"), QuestScriptTable ? TEXT("성공") : TEXT("실패 - QuestSettings 확인 필요"));
		UE_LOG(LogTemp, Warning, TEXT("QuestAnswerTable 로드 %s"), QuestAnswerTable ? TEXT("성공") : TEXT("실패 - QuestSettings 확인 필요"));
		UE_LOG(LogTemp, Warning, TEXT("RandomQuestAnswerTable 로드 %s"), RandomQuestAnswerTable ? TEXT("성공") : TEXT("실패 - QuestSettings 확인 필요"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UQuestSettings를 찾을 수 없습니다."));
	}
}


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
	UE_LOG(LogTemp, Warning, TEXT("퀘스트 수락: %s"), *QuestID.ToString());
}

// 특정 퀘스트의 현재 진행 상태(수락 전/수락함/완료함)를 조회
// 한 번도 언급되지 않은 QuestID는 기본값으로 NotAccepted를 반환
EQuestState UQuestManager::GetQuestState(FName QuestID) const
{
	if (QuestStates.Contains(QuestID))
	{
		return QuestStates[QuestID];
	}
	return EQuestState::NotAccepted;
}

// 저널용 퀘스트 전체 조회 함수 (수락된 순서 그대로 반환)
TArray<FName> UQuestManager::GetAllTrackedQuestIDs() const
{
	return QuestOrder;
}

// 이 QuestID가 RandomQuestAnswerTable 소속인지 확인 (일반 테이블에 있으면 false)
bool UQuestManager::IsRandomQuest(FName QuestID) const
{
	bool bIsRandom = false;
	FindAnswerData(QuestID, &bIsRandom);
	return bIsRandom;
}

// 저널 탭 분리용 - 일반 퀘스트만 필터링해서 반환
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

// 저널 탭 분리용 - 랜덤 퀘스트만 필터링해서 반환
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

// 팀원 요청으로 분리된 완료 처리 함수 (TryDeliver 등 여러 곳에서 재사용 가능)
void UQuestManager::CompleteQuest(FName QuestID)
{
	if (QuestID.IsNone()) return;

	if (GetQuestState(QuestID) == EQuestState::Completed)
	{
		return;
	}

	QuestStates.Add(QuestID, EQuestState::Completed);
	OnQuestUpdated.Broadcast(QuestID, EQuestState::Completed);
	UE_LOG(LogTemp, Warning, TEXT("퀘스트 [%s] 완료 처리"), *QuestID.ToString());
}

// ===================================================================
// [텍스트 조회 - UI 전용 참조 함수]
// DT_QuestScript(텍스트 전용 DataTable)에서 값을 가져옴
// 아래 함수들은 전부 텍스트만 반환하며, 정답 판정용 데이터(RequestedEffects)에는
// 접근하지 않으므로 UI에서 안전하게 자유롭게 호출 가능
// ===================================================================

// 퀘스트 제목 조회 (저널 목록용)
FText UQuestManager::GetQuestTitle(FName QuestID) const
{
	if (!QuestScriptTable) return FText::GetEmpty();

	FQuestData* Quest = QuestScriptTable->FindRow<FQuestData>(QuestID, TEXT(""));
	return Quest ? Quest->QuestTitle : FText::GetEmpty();
}

// 마을 NPC가 퀘스트를 제안할 때 보여줄 원문 대사
TArray<FText> UQuestManager::GetQuestScriptLines(FName QuestID) const
{
	if (!QuestScriptTable) return TArray<FText>();
	FQuestData* Quest = QuestScriptTable->FindRow<FQuestData>(QuestID, TEXT(""));
	return Quest ? Quest->QuestScriptLines : TArray<FText>();
}

// 퀘스트 수락 후, 저널/퀘스트 목록 UI에서 다시 확인할 때 보여줄 요약 텍스트
FText UQuestManager::GetQuestSummaryText(FName QuestID) const
{
	if (!QuestScriptTable) return FText::GetEmpty();

	FQuestData* Quest = QuestScriptTable->FindRow<FQuestData>(QuestID, TEXT(""));
	return Quest ? Quest->QuestSummaryText : FText::GetEmpty();
}

FText UQuestManager::GetQuestScriptTextJoined(FName QuestID) const
{
	TArray<FText> Lines = GetQuestScriptLines(QuestID);
	return FText::Join(FText::FromString(TEXT("\n")), Lines);
}

// ===================================================================
// [세션 힌트 / NPC 스토리]
// GetSessionHintText : 1차 힌트 (레벨 0)
// GetNPCStoryLines : 힌트 열람 후 보여줄 NPC 스토리 (레벨 1) - 단일/배열 버전
// GetQuestHintLevel / SetQuestHintLevel : 현재 열람 단계 조회/갱신
// ===================================================================

// 1차 힌트
FText UQuestManager::GetSessionHintText(FName QuestID) const
{
	FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->SessionHintText : FText::GetEmpty();
}



// 특정 퀘스트가 현재 몇 번째 힌트 단계까지 열람했는지 조회 (0: 기본, 1: 2차, 2: 3차)
int32 UQuestManager::GetQuestHintLevel(FName QuestID) const
{
	if (const int32* Level = QuestHintLevels.Find(QuestID))
	{
		return *Level;
	}
	return 0;
}

// 힌트 단계 갱신 (더보기 버튼 클릭 시 UI가 호출)
void UQuestManager::SetQuestHintLevel(FName QuestID, int32 NewLevel)
{
	QuestHintLevels.Add(QuestID, NewLevel);
	UE_LOG(LogTemp, Log, TEXT("퀘스트 %s 힌트 단계 %d(으)로 갱신"), *QuestID.ToString(), NewLevel);
}


TArray<FText> UQuestManager::GetNPCStoryLines(FName QuestID) const
{
	FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->NPCStoryLines : TArray<FText>();
}

// 퀘스트 정답 찾기 함수 - QuestAnswerTable(고유)을 먼저 찾고, 없으면 RandomQuestAnswerTable(랜덤)에서 찾음
// bOutIsRandom이 주어지면 어느 테이블에서 찾았는지 결과를 같이 반환
FQuestAnswerData* UQuestManager::FindAnswerData(FName QuestID, bool* bOutIsRandom) const
{
	if (QuestAnswerTable)
	{
		if (FQuestAnswerData* Found = QuestAnswerTable->FindRow<FQuestAnswerData>(QuestID, TEXT("")))
		{
			if (bOutIsRandom) *bOutIsRandom = false;
			return Found;
		}
	}
	if (RandomQuestAnswerTable)
	{
		if (FQuestAnswerData* Found = RandomQuestAnswerTable->FindRow<FQuestAnswerData>(QuestID, TEXT("")))
		{
			if (bOutIsRandom) *bOutIsRandom = true;
			return Found;
		}
	}
	if (bOutIsRandom) *bOutIsRandom = false;
	return nullptr;
}

// 퀘스트 완료 보상 골드 조회
int32 UQuestManager::GetRewardGold(FName QuestID) const
{
	FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->RewardGold : 0;
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
// [납품 판정 - 퍼즐 시스템과의 연결 지점]
// ===================================================================

// - PotionResult: 퍼즐 시스템이 만든 최종 재료 태그 배열
//   → 퍼즐 시스템이 이 결과물을 "어떻게" 만들었는지는 이 함수가 알 필요 없음
//     오직 이 형태(태그 배열)로 결과가 들어오기만 하면 됨
// - Fail이 아닌 경우 CompleteQuest()를 호출해 퀘스트를 완료 처리하고, 보상 골드를 방송함
EDeliveryGrade UQuestManager::TryDeliver(FName QuestID, const TArray<FGameplayTag>& PotionResult)
{
	if (GetQuestState(QuestID) == EQuestState::Completed)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 완료 처리된 퀘스트입니다: %s"), *QuestID.ToString());
		return EDeliveryGrade::Fail;
	}

	FQuestAnswerData* Answer = FindAnswerData(QuestID);
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

	EDeliveryGrade Grade = bAllTagsMatched ? EDeliveryGrade::Perfect : EDeliveryGrade::Fail;

	if (Grade == EDeliveryGrade::Perfect)
	{
		CompleteQuest(QuestID);

		int32 Gold = GetRewardGold(QuestID);
		OnRewardGranted.Broadcast(Gold);
	}

	UE_LOG(LogTemp, Warning, TEXT("퀘스트 %s 납품 결과: %s"), *QuestID.ToString(), bAllTagsMatched ? TEXT("전체 충족") : TEXT("일부 누락"));

	return Grade;
}

// 조건 하나하나에 대한 세부 판정 (태그정답/태그오답)
// - 1단계: 정답이 요구하는 조건들을 기준으로, 포션이 각각 만족했는지 확인
// - 2단계: 포션에 들어있는 태그 중, 정답이 요구하지 않은 "쓸데없는 태그"가 섞였는지 확인
//   (정답 개수만 기준으로 돌면, 포션에 여분으로 섞인 태그를 놓치는 문제가 있어 추가함)
TArray<FConditionEvaluation> UQuestManager::EvaluateConditions(FName QuestID, const TArray<FGameplayTag>& PotionResult) const
{
	TArray<FConditionEvaluation> Results;

	FQuestAnswerData* Answer = FindAnswerData(QuestID);
	if (!Answer) return Results;

	// 1. 정답 기준으로 순회 - 요구한 각 조건이 맞는지 확인
	for (const FQuestEffectRequirement& Req : Answer->RequestedEffects)
	{
		FConditionEvaluation Eval;
		Eval.Axis = Req.Axis;
		Eval.bWasRequested = true;

		if (!PotionResult.Contains(Req.Axis))
		{
			Eval.Result = EConditionMatchResult::WrongTag;
		}
		else
		{
			Eval.Result = EConditionMatchResult::Correct;
		}

		Results.Add(Eval);
	}

	// 2. 포션 기준으로 순회 - 정답에 없는데 플레이어가 넣은 태그(잡내)를 찾아냄
	for (const FGameplayTag& EffectTag : PotionResult)
	{
		bool bIsRequested = Answer->RequestedEffects.ContainsByPredicate(
			[&](const FQuestEffectRequirement& Req) { return Req.Axis == EffectTag; }
		);

		if (!bIsRequested)
		{
			FConditionEvaluation Eval;
			Eval.Axis = EffectTag;
			Eval.Result = EConditionMatchResult::WrongTag;
			Eval.bWasRequested = false;  // 여분 태그임을 표시

			Results.Add(Eval);
		}
	}

	return Results;
}

// 특정 퀘스트가 요구하는 조건 목록 원본을 그대로 반환 (필요 시 외부에서 직접 순회하고 싶을 때 사용)
TArray<FQuestEffectRequirement> UQuestManager::GetQuestEffectRequirements(FName QuestID) const
{
	FQuestAnswerData* Answer = FindAnswerData(QuestID);
	if (!Answer) return TArray<FQuestEffectRequirement>();

	return Answer->RequestedEffects;
}

// 반응 텍스트 배열에서 랜덤으로 하나 선택. 배열이 비어있으면 빈 텍스트 반환.
FText UQuestManager::PickRandomReaction(const TArray<FText>& Reactions) const
{
	if (Reactions.Num() == 0) return FText::GetEmpty();

	int32 RandomIndex = FMath::RandRange(0, Reactions.Num() - 1);
	return Reactions[RandomIndex];
}

// 조건 판정 결과(Axis + Result + bWasRequested)에 맞는 NPC 반응 대사를 랜덤으로 반환
// - 여분 태그(bWasRequested == false): 퀘스트 공용 OnWrongTagReactions에서 선택
// - 요청된 태그 충족(Correct): 해당 축의 OnMatchReactions에서 선택
// - 요청된 태그 누락(WrongTag, bWasRequested == true): 해당 축의 OnMissingReactions에서 선택
FText UQuestManager::GetReactionText(FName QuestID, const FConditionEvaluation& Evaluation) const
{
	FQuestAnswerData* Answer = FindAnswerData(QuestID);
	if (!Answer) return FText::GetEmpty();

	if (!Evaluation.bWasRequested)
	{
		return PickRandomReaction(Answer->OnWrongTagReactions);
	}

	const FQuestEffectRequirement* MatchingReq = Answer->RequestedEffects.FindByPredicate(
		[&](const FQuestEffectRequirement& Req) { return Req.Axis == Evaluation.Axis; }
	);
	if (!MatchingReq) return FText::GetEmpty();

	if (Evaluation.Result == EConditionMatchResult::Correct)
	{
		return PickRandomReaction(MatchingReq->OnMatchReactions);
	}
	else
	{
		return PickRandomReaction(MatchingReq->OnMissingReactions);
	}
}

// ===================================================================
// [무결성 검증 - 개발 중 확인용]
// ===================================================================

// DT_QuestScript(텍스트)와 DT_QuestAnswer(정답)는 서로 다른 DataTable로 분리되어 있어
// 반드시 같은 QuestID(Row Name)로 짝이 맞아야 함
// 한쪽에만 있고 다른 쪽에 없는 QuestID가 있으면 에러 로그로 알려줌
// 프로토타입 기준 두 DT 모두 에디터에서 관리하나, 이후 텍스트 DT는 CSV로, 정답 DT는 에디터에서 관리할 예정
void UQuestManager::ValidateQuestTablesMatch()
{
	if (!QuestScriptTable || !QuestAnswerTable) return;

	TArray<FName> ScriptIDs = QuestScriptTable->GetRowNames();
	TArray<FName> AnswerIDs = QuestAnswerTable->GetRowNames();

	for (const FName& ID : ScriptIDs)
	{
		if (!AnswerIDs.Contains(ID))
		{
			UE_LOG(LogTemp, Error, TEXT("퀘스트 %s 의 정답 데이터(QuestAnswerTable)가 없습니다!"), *ID.ToString());
		}
	}

	for (const FName& ID : AnswerIDs)
	{
		if (!ScriptIDs.Contains(ID))
		{
			UE_LOG(LogTemp, Error, TEXT("퀘스트 %s 의 텍스트 데이터(QuestScriptTable)가 없습니다!"), *ID.ToString());
		}
	}
}
