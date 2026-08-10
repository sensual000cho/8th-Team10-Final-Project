// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/Quest/QuestManager.h"

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

	QuestStates.Add(QuestID, EQuestState::Accepted);
	OnQuestUpdated.Broadcast(QuestID, EQuestState::Accepted);
	UE_LOG(LogTemp, Warning, TEXT("퀘스트 수락: %s"), *QuestID.ToString());

	if (!AcceptedQuestOrder.Contains(QuestID))
	{
		AcceptedQuestOrder.Add(QuestID);
	}
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

/* =================================================================
[텍스트 조회 - UI 전용 참조 함수]
DT_QuestScript(텍스트 전용 DataTable)에서 값을 가져옴
아래 4개 함수는 전부 텍스트만 반환하며, 정답 판정용 데이터(RequestedEffects)에는
접근하지 않으므로 UI에서 안전하게 자유롭게 호출 가능
===================================================================*/

// 퀘스트 제목 조회
FText UQuestManager::GetQuestTitle(FName QuestID) const
{
	if (!QuestScriptTable) return FText::GetEmpty();

	FQuestData* Quest = QuestScriptTable->FindRow<FQuestData>(QuestID, TEXT(""));
	return Quest ? Quest->QuestTitle : FText::GetEmpty();
}
// 마을 NPC가 퀘스트를 제안할 때 보여줄 원문 대사
FText UQuestManager::GetQuestFullText(FName QuestID) const
{
	if (!QuestScriptTable) return FText::GetEmpty();

	FQuestData* Quest = QuestScriptTable->FindRow<FQuestData>(QuestID, TEXT(""));
	return Quest ? Quest->QuestText_Full : FText::GetEmpty();
}

// 퀘스트 수락 후, 저널/퀘스트 목록 UI에서 다시 확인할 때 보여줄 요약 텍스트
FText UQuestManager::GetQuestSummaryText(FName QuestID) const
{
	if (!QuestScriptTable) return FText::GetEmpty();

	FQuestData* Quest = QuestScriptTable->FindRow<FQuestData>(QuestID, TEXT(""));
	return Quest ? Quest->QuestText_Summary : FText::GetEmpty();
}

// 공방 포션 제조 세션 진입 시 보여줄 1차 힌트(서술형, 강도 표현 포함)
// 예: "잠은 아주아주 잘 오면 좋겠고, 몸은 적당히 따뜻하면 좋겠어."
// 실제 판정 수치(RequestedEffects)는 노출하지 않음
FText UQuestManager::GetSessionHintText(FName QuestID) const
{
	if (!QuestAnswerTable) return FText::GetEmpty();

	FQuestAnswerData* Answer = QuestAnswerTable->FindRow<FQuestAnswerData>(QuestID, TEXT(""));
	return Answer ? Answer->SessionHintText : FText::GetEmpty();
}

// "네? 그게 뭐죠?" 같은 추가 힌트 버튼을 눌렀을 때 보여줄 2차(상세) 힌트
// SessionHintText보다 조금 더 구체적이지만 여전히 숫자는 노출하지 않음
FText UQuestManager::GetSessionHintTextDetailed(FName QuestID) const
{
	if (!QuestAnswerTable) return FText::GetEmpty();

	FQuestAnswerData* Answer = QuestAnswerTable->FindRow<FQuestAnswerData>(QuestID, TEXT(""));
	return Answer ? Answer->SessionHintText_Detailed : FText::GetEmpty();
}

// 최종 힌트 
FText UQuestManager::GetSessionHintTextDetailed2(FName QuestID) const
{
	if (!QuestAnswerTable) return FText::GetEmpty();

	FQuestAnswerData* Answer = QuestAnswerTable->FindRow<FQuestAnswerData>(QuestID, TEXT(""));
	return Answer ? Answer->SessionHintText_Detailed2 : FText::GetEmpty();
}

// 저널용 퀘스트 전체 조회 함수
TArray<FName> UQuestManager::GetAllTrackedQuestIDs() const
{
	TArray<FName> Result;
	QuestStates.GetKeys(Result);
	return Result;
}

// ===================================================================
// 무결성 검증 - 개발 중 확인용
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
}

// ===================================================================
// [납품 판정 - 퍼즐 시스템과의 연결 지점]
// ===================================================================

// 퍼즐(조제) 담당자가 완성한 최종 결과물(PotionResult)을 받아서,
// 해당 퀘스트가 요구하는 조건(RequestedEffects)을 몇 개나 만족했는지 확인하고
// 등급(Fail/Okay/Good/Perfect)을 넘겨주는 판정 함수
//
// - PotionResult: 퍼즐 시스템이 만든 최종 재료 태그+수치 배열 (TArray<FAlchemyProperty>)
//   → 퍼즐 시스템이 이 결과물을 "어떻게" 만들었는지는 이 함수가 알 필요 없음
//     오직 이 형태(태그+값 배열)로 결과가 들어오기만 하면 됨
// - 판정 로직: 각 조건(Axis, MinValue, MaxValue)에 대해, PotionResult 안에 같은 Tag를 가진
//   항목이 있고 그 값이 범위 안에 들어오는지 확인
// - Fail이 아닌 경우(즉 한 개 이상 조건을 만족한 경우) 퀘스트 상태를 Completed로 전환하고
//   OnQuestUpdated 델리게이트를 방송함
EDeliveryGrade UQuestManager::TryDeliver(FName QuestID, const TArray<FAlchemyProperty>& PotionResult)
{
	if (GetQuestState(QuestID) == EQuestState::Completed)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 완료 처리된 퀘스트입니다: %s"), *QuestID.ToString());
		return EDeliveryGrade::Fail;
	}

	if (!QuestAnswerTable) return EDeliveryGrade::Fail;

	FQuestAnswerData* Answer = QuestAnswerTable->FindRow<FQuestAnswerData>(QuestID, TEXT(""));
	if (!Answer) return EDeliveryGrade::Fail;

	int32 CorrectCount = 0;
	int32 TotalCount = Answer->RequestedEffects.Num();

	for (const FQuestEffectRequirement& Req : Answer->RequestedEffects)
	{
		// PotionResult 배열에서, 이 조건이 요구하는 태그(Axis)와 같은 항목을 찾음
		const FAlchemyProperty* Matching = PotionResult.FindByPredicate(
			[&](const FAlchemyProperty& P) { return P.Tag == Req.Axis; }
		);

		// 찾았고, 그 값이 요구 범위(MinValue~MaxValue) 안에 들어오면 조건 만족으로 카운트
		if (Matching && Matching->Value >= Req.MinValue && Matching->Value <= Req.MaxValue)
		{
			CorrectCount++;
		}
	}

	// 만족한 조건 개수에 따라 등급 결정
	// - 0개: Fail
	// - 전부: Perfect
	// - 전체-1개 이상: Good
	// - 그 외: Okay
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

	// Fail이 아니면(즉 납품이 성립하면) 퀘스트를 완료 상태로 전환하고 알림 방송
	if (Grade != EDeliveryGrade::Fail)
	{
		QuestStates.Add(QuestID, EQuestState::Completed);
		OnQuestUpdated.Broadcast(QuestID, EQuestState::Completed);
	}

	UE_LOG(LogTemp, Warning, TEXT("퀘스트 %s 납품 결과: %d/%d 조건 만족"), *QuestID.ToString(), CorrectCount, TotalCount);

	return Grade;
}

// 최종 결과 점수 반영용 평가 함수
TArray<EConditionMatchResult> UQuestManager::EvaluateConditions(FName QuestID, const TArray<FAlchemyProperty>& PotionResult) const
{
	TArray<EConditionMatchResult> Results;

	if (!QuestAnswerTable) return Results;

	FQuestAnswerData* Answer = QuestAnswerTable->FindRow<FQuestAnswerData>(QuestID, TEXT(""));
	if (!Answer) return Results;

	for (const FQuestEffectRequirement& Req : Answer->RequestedEffects)
	{
		const FAlchemyProperty* Matching = PotionResult.FindByPredicate(
			[&](const FAlchemyProperty& P) { return P.Tag == Req.Axis; }
		);

		if (!Matching)
		{
			Results.Add(EConditionMatchResult::WrongTag);
		}
		else if (Matching->Value < Req.MinValue)
		{
			Results.Add(EConditionMatchResult::TooLow);
		}
		else if (Matching->Value > Req.MaxValue)
		{
			Results.Add(EConditionMatchResult::TooHigh);
		}
		else
		{
			Results.Add(EConditionMatchResult::Correct);
		}
	}

	return Results;
}

// ===================================================================
// [테스트 전용 함수 - 실제 게임 로직 아님]
// ===================================================================

// TryDeliver()가 올바르게 동작하는지 검증하기 위한 임시 테스트 함수.
// 실제 게임에서는 퍼즐(조제) 시스템이 PotionResult를 만들어서 TryDeliver에 넘겨주므로,
// 이 함수는 그 역할을 손으로 흉내 낸 것뿐임.
// ※ 실제 퍼즐 시스템 연동이 완료되면 이 함수는 삭제 예정.
void UQuestManager::TestTryDeliver()
{
	TArray<FAlchemyProperty> TestPotion;

	// 가짜 조제 결과 1: 잠(Drowsiness) 축 2단계
	FAlchemyProperty Prop1;
	Prop1.Tag = FGameplayTag::RequestGameplayTag(FName("Alchemy.Drowsiness"));
	Prop1.Value = 2;
	TestPotion.Add(Prop1);

	// 가짜 조제 결과 2: 체온(BodyHeat) 축 1단계
	FAlchemyProperty Prop2;
	Prop2.Tag = FGameplayTag::RequestGameplayTag(FName("Alchemy.BodyHeat"));
	Prop2.Value = 1;
	TestPotion.Add(Prop2);

	EDeliveryGrade Grade = TryDeliver(FName("Origin_Q001"), TestPotion);
	UE_LOG(LogTemp, Warning, TEXT("테스트 결과 등급: %d"), (int32)Grade);
}

#pragma region 리퀘스트 힌트 단계 저장 관련
int32 UQuestManager::GetQuestHintLevel(FName QuestID) const
{
	// 맵에 기록된 힌트 단계가 있다면 반환, 없다면 0단계(처음) 반환
	if (const int32* Level = QuestHintLevels.Find(QuestID))
	{
		return *Level;
	}
	return 0;
}

void UQuestManager::SetQuestHintLevel(FName QuestID, int32 NewLevel)
{
	// 새로운 힌트 단계 저장 (기존에 있으면 덮어쓰기)
	QuestHintLevels.Add(QuestID, NewLevel);
	UE_LOG(LogTemp, Log, TEXT("퀘스트 %s 힌트 단계 %d(으)로 갱신"), *QuestID.ToString(), NewLevel);
}

FText UQuestManager::GetCurrentSessionHintText(FName QuestID) const
{
	if (!QuestAnswerTable) return FText::GetEmpty();

	FQuestAnswerData* Answer = QuestAnswerTable->FindRow<FQuestAnswerData>(QuestID, TEXT(""));
	if (!Answer) return FText::GetEmpty();

	// 저장된 힌트 단계를 확인해서 알맞은 대사를 반환
	int32 Level = GetQuestHintLevel(QuestID);

	if (Level == 1)
	{
		return Answer->SessionHintText_Detailed;
	}
	else if (Level >= 2)
	{
		return Answer->SessionHintText_Detailed2; 
	}

	// Level이 0이거나 그 외의 경우 기본 힌트
	return Answer->SessionHintText;
}
#pragma endregion