#include "GameMode/CPLabGameMode.h"

#include "Engine/Engine.h"
#include "GameState/CPLabGameState.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"
#include "PlayerState/CPLabPlayerState.h"

namespace
{
	// 실제 리퀘스트 시스템이 연결되기 전 사용할 간단한 테스트 데이터 생성
	FCPLabPotionRequest MakeTestRequest(
		FName RequestId,
		FText DisplayText)
	{
		FCPLabPotionRequest PotionRequest;
		PotionRequest.RequestId = RequestId;
		PotionRequest.DisplayText = MoveTemp(DisplayText);
		return PotionRequest;
	}
}

ACPLabGameMode::ACPLabGameMode()
{
	GameStateClass = ACPLabGameState::StaticClass();
	PlayerStateClass = ACPLabPlayerState::StaticClass();

	// 현재 프로토타입은 리퀘스트 하나로 전체 제조 흐름 확인
	DefaultTestRequests =
	{
		MakeTestRequest(
			FName(TEXT("TestPotionRequest01")),
			FText::FromString(TEXT("Create the first test potion.")))
	};
}

bool ACPLabGameMode::TryStartLabSession()
{
	return TryStartLabSessionWithRequests(DefaultTestRequests);
}

bool ACPLabGameMode::TryStartLabSessionWithRequests(const TArray<FCPLabPotionRequest>& PotionRequests)
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return false;
	
	const bool bStarted = Session->StartSession(PotionRequests);
	if (bStarted && GEngine){
		FCPLabPotionRequestState ActiveRequestState;
		if (!Session->GetActiveRequestState(ActiveRequestState)){
			GEngine->AddOnScreenDebugMessage(
				-1, 3.0f, FColor::Red, TEXT("[Lab] 활성화된 리퀘스트가 없습니다."));
			return bStarted;
		}
		GEngine->AddOnScreenDebugMessage(
			-1, 3.0f, FColor::Cyan, 
			FString::Printf(TEXT("[Lab] %s"), *ActiveRequestState.PotionRequest.DisplayText.ToString()));
	}
	return bStarted;
}

void ACPLabGameMode::ResetLabSession()
{
	if (UCPLabPotionSessionComponent* Session = GetPotionSession())
	{
		Session->ResetSession();
	}
}

bool ACPLabGameMode::TrySelectRequest(FName RequestId)
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session && Session->TrySetActiveRequest(RequestId);
}

bool ACPLabGameMode::TryAcceptActiveRequest()
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session &&
		Session->TrySetRequestPhase(
			GetActiveRequestId(),
			ECPLabPotionRequestPhase::Preparing);
}

bool ACPLabGameMode::TryBeginActiveRequestProcessing()
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session &&
		Session->TrySetRequestPhase(
			GetActiveRequestId(),
			ECPLabPotionRequestPhase::Processing);
}

bool ACPLabGameMode::TryFinishActivePotion()
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session &&
		Session->TrySetRequestPhase(
			GetActiveRequestId(),
			ECPLabPotionRequestPhase::PotionReady);
}

bool ACPLabGameMode::TryDeliverActivePotion()
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session &&
		Session->TryMarkRequestDelivered(GetActiveRequestId());
}

bool ACPLabGameMode::TryPlaceIngredient(
	FName RequestId,
	int32 SlotIndex,
	const FCPLabIngredientInstance& Ingredient)
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session &&
		Session->TryPlaceIngredient(
			RequestId,
			SlotIndex,
			Ingredient);
}

bool ACPLabGameMode::TryClearIngredient(
	FName RequestId,
	int32 SlotIndex)
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session &&
		Session->TryClearIngredient(RequestId, SlotIndex);
}

bool ACPLabGameMode::TryCreateWorkingIngredient(
	FName RequestId,
	int32 SlotIndex,
	FCPLabIngredientInstance& OutWorkingIngredient) const
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session && Session->TryCreateWorkingCopy(
		RequestId,
		SlotIndex,
		OutWorkingIngredient);
}

bool ACPLabGameMode::TryCommitWorkingIngredient(
	FName RequestId,
	int32 SlotIndex,
	const FCPLabIngredientInstance& WorkingIngredient)
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session && Session->TryCommitWorkingCopy(
		RequestId,
		SlotIndex,
		WorkingIngredient);
}

void ACPLabGameMode::DebugAdvanceSessionPhase()
{
	// 월드 Actor가 완성되기 전 세션 진행 상태만 빠르게 확인하는 함수
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return;

	const FCPLabPotionSessionState SessionState = Session->GetSessionState();

	if (SessionState.Phase == ECPLabPotionSessionPhase::WaitingForBell){
		const bool bStarted = TryStartLabSession();
		return;
	}else if (SessionState.Phase == ECPLabPotionSessionPhase::Completed){
		ResetLabSession();
		return;
	}

	FCPLabPotionRequestState ActiveRequestState;
	if (!Session->GetActiveRequestState(ActiveRequestState)) return;

	switch (ActiveRequestState.Phase){
	case ECPLabPotionRequestPhase::Queued:
		TryAcceptActiveRequest();
		break;

	case ECPLabPotionRequestPhase::Preparing:
		TryBeginActiveRequestProcessing();
		break;

	case ECPLabPotionRequestPhase::Processing:
		TryFinishActivePotion();
		break;

	case ECPLabPotionRequestPhase::PotionReady:
		TryDeliverActivePotion();
		break;

	default:
		break;
	}
}

ACPLabGameState* ACPLabGameMode::GetLabGameState() const
{
	return Cast<ACPLabGameState>(GameState);
}

UCPLabPotionSessionComponent*
ACPLabGameMode::GetPotionSession() const
{
	const ACPLabGameState* LabState = GetLabGameState();
	return LabState ? LabState->GetPotionSession() : nullptr;
}

FName ACPLabGameMode::GetActiveRequestId() const
{
	const UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session)
	{
		return NAME_None;
	}

	FCPLabPotionRequestState ActiveRequestState;
	return Session->GetActiveRequestState(ActiveRequestState)
		? ActiveRequestState.PotionRequest.RequestId
		: NAME_None;
}
