// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/Quest/QuestManager.h"

// ===================================================================
// [초기화 - GameInstanceSubsystem 생성 시 자동 호출]
// ===================================================================

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// DT_QuestScript 로드 (원문/요약 텍스트 전용)
	UDataTable* LoadedScriptTable = Cast<UDataTable>(
		StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/CreatePotion/Quest/DT_QuestScript.DT_QuestScript"))
	);
	if (LoadedScriptTable)
	{
		QuestScriptTable = LoadedScriptTable;
		UE_LOG(LogTemp, Warning, TEXT("QuestScriptTable 로드 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("QuestScriptTable 로드 실패 - 경로 확인 필요"));
	}

	// DT_QuestAnswer 로드 (조건/정답 + 세션 힌트 전용, Hidden 카테고리 포함)
	UDataTable* LoadedAnswerTable = Cast<UDataTable>(
		StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/CreatePotion/Quest/DT_QuestAnswer.DT_QuestAnswer"))
	);
	if (LoadedAnswerTable)
	{
		QuestAnswerTable = LoadedAnswerTable;
		UE_LOG(LogTemp, Warning, TEXT("QuestAnswerTable 로드 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("QuestAnswerTable 로드 실패 - 경로 확인 필요"));
	}

	// TODO: DT_RandomQuestAnswer 테이블 생성 후 주석 해제 및 경로 확인
	
	UDataTable* LoadedRandomAnswerTable = Cast<UDataTable>(
		StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/CreatePotion/Quest/DT_RandomQuestAnswer.DT_RandomQuestAnswer"))
	);
	if (LoadedRandomAnswerTable)
	{
		RandomQuestAnswerTable = LoadedRandomAnswerTable;
		UE_LOG(LogTemp, Warning, TEXT("RandomQuestAnswerTable 로드 성공"));
	}
	
}

// ===================================================================
// [퀘스트 수락/상태 관리]
// ===================================================================

/* NPC 대화 UI에서 "수락" 버튼을 눌렀을 때 호출
 퀘스트 상태를 Accepted로 기록하고, OnQuestUpdated 델리게이트를 방송하여
 이를 구독 중인 UI(퀘스트 저널 등)가 자동으로 갱신*/
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
	return Quest ? Quest->QuestText_Summary : FText::GetEmpty();
}

//임시 연결용 함수
FText UQuestManager::GetQuestFullTextJoined(FName QuestID) const
{
	TArray<FText> Lines = GetQuestScriptLines(QuestID);
	return FText::Join(FText::FromString(TEXT(" ")), Lines);
}

// ===================================================================
// [세션 힌트 - 단계별 조회 및 현재 단계 자동 관리]
// DT_QuestAnswer에서 값을 가져옴. 실제 판정 수치(RequestedEffects)는 노출하지 않음.
// - GetSessionHintText / Detailed / Detailed2 : 단계별 개별 조회 (저수준)
// - GetCurrentSessionHintText : 저장된 현재 단계에 맞는 힌트를 자동으로 골라 반환 (UI 권장 사용)
// ===================================================================

// 1차 힌트
FText UQuestManager::GetSessionHintText(FName QuestID) const
{
	FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->SessionHintText : FText::GetEmpty();
}

// 2차(상세) 힌트
FText UQuestManager::GetSessionHintTextDetailed(FName QuestID) const
{
	FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->SessionHintText_Detailed : FText::GetEmpty();
}

// 3차(최종) 힌트
FText UQuestManager::GetSessionHintTextDetailed2(FName QuestID) const
{
	FQuestAnswerData* Answer = FindAnswerData(QuestID);
	return Answer ? Answer->SessionHintText_Detailed2 : FText::GetEmpty();
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

// 저장된 힌트 단계에 맞는 텍스트를 자동으로 골라 반환 (UI는 이 함수 하나만 호출하면 됨)
FText UQuestManager::GetCurrentSessionHintText(FName QuestID) const
{
	FQuestAnswerData* Answer = FindAnswerData(QuestID);
	if (!Answer) return FText::GetEmpty();

	int32 Level = GetQuestHintLevel(QuestID);

	if (Level == 1)
	{
		return Answer->SessionHintText_Detailed;
	}
	else if (Level >= 2)
	{
		return Answer->SessionHintText_Detailed2;
	}

	return Answer->SessionHintText;
}

// 퀘스트 정답 찾기 함수 - QuestAnswerTable(고유)을 먼저 찾고, 없으면 RandomQuestAnswerTable(랜덤)에서 찾음
FQuestAnswerData* UQuestManager::FindAnswerData(FName QuestID) const
{
	if (QuestAnswerTable)
	{
		if (FQuestAnswerData* Found = QuestAnswerTable->FindRow<FQuestAnswerData>(QuestID, TEXT("")))
			return Found;
	}
	if (RandomQuestAnswerTable)
	{
		if (FQuestAnswerData* Found = RandomQuestAnswerTable->FindRow<FQuestAnswerData>(QuestID, TEXT("")))
			return Found;
	}
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
	if (AllIDs.Num() == 0) return NAME_None;

	int32 RandomIndex = FMath::RandRange(0, AllIDs.Num() - 1);
	return AllIDs[RandomIndex];
}

// ===================================================================
// [납품 판정 - 퍼즐 시스템과의 연결 지점]
// ===================================================================

// 퍼즐(조제) 담당자가 완성한 최종 결과물(PotionResult)을 받아서,
// 해당 퀘스트가 요구하는 조건(RequestedEffects)을 몇 개나 만족했는지 확인하고
// 등급(Fail/Okay/Good/Perfect)을 넘겨주는 판정 함수
//
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

	int32 CorrectCount = 0;
	int32 TotalCount = Answer->RequestedEffects.Num();

	for (const FQuestEffectRequirement& Req : Answer->RequestedEffects)
	{
		if (PotionResult.Contains(Req.Axis))
		{
			CorrectCount++;
		}
	}

	// 만족한 조건 개수에 따라 등급 결정
	// ※ 등급 기준(배율/개수)은 추후 밸런스 조정 시 변경 가능
	EDeliveryGrade Grade = EDeliveryGrade::Fail;
	if (CorrectCount == 0)
	{
		Grade = EDeliveryGrade::Fail;
	}
	else if (CorrectCount == TotalCount)
	{
		Grade = EDeliveryGrade::Perfect;
	}
	else if (CorrectCount >= TotalCount - 1)
	{
		Grade = EDeliveryGrade::Good;
	}
	else
	{
		Grade = EDeliveryGrade::Okay;
	}

	if (Grade != EDeliveryGrade::Fail)
	{
		CompleteQuest(QuestID);

		int32 Gold = GetRewardGold(QuestID);
		OnRewardGranted.Broadcast(Gold);
	}

	UE_LOG(LogTemp, Warning, TEXT("퀘스트 %s 납품 결과: %d/%d 조건 만족"), *QuestID.ToString(), CorrectCount, TotalCount);

	return Grade;
}

// 조건 하나하나에 대한 세부 판정 (O/Up/Down/태그오답)
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
