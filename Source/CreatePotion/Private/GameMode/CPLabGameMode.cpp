// CPLabGameMode.cpp

#include "GameMode/CPLabGameMode.h"

#include "GameState/CPLabGameState.h"
#include "PlayerState/CPLabPlayerState.h"
//#include "UI/CPLabHUD.h"

ACPLabGameMode::ACPLabGameMode()
{
	GameStateClass = ACPLabGameState::StaticClass();
	PlayerStateClass = ACPLabPlayerState::StaticClass();
	//UI 쪽 변경/확정 전까진 주석처리
	//HUDClass = ACPLabHUD::StaticClass();
	DefaultTestRequest.RequestId = FName(TEXT("TestSleepWarmCalm"));
	DefaultTestRequest.DisplayText = FText::FromString(
		TEXT("잠이 잘 오고, 몸이 따뜻해지며, 마음이 편안해지는 물약을 만들어 주세요."));
}

ACPLabGameState* ACPLabGameMode::GetLabGameState() const
{
	return Cast<ACPLabGameState>(GameState);
}

bool ACPLabGameMode::SetSessionPhase(ECPLabSessionPhase NewPhase)
{
	if (ACPLabGameState* LabState = GetLabGameState())
	{
		return LabState->ApplySessionPhase(NewPhase);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Lab] CPLabGameState was not created."));

	return false;
}

void ACPLabGameMode::ResetLabSession()
{
	if (ACPLabGameState* LabState = GetLabGameState())
	{
		LabState->ClearRequest();
	}

	SetSessionPhase(ECPLabSessionPhase::WaitingForBell);
}

bool ACPLabGameMode::TryStartLabSession()
{
	return TryStartLabSessionWithRequest(DefaultTestRequest);
}

bool ACPLabGameMode::TryStartLabSessionWithRequest(const FCPLabRequest& Request)
{
	ACPLabGameState* LabState = GetLabGameState();
	if (!LabState ||
		LabState->GetCurrentPhase() != ECPLabSessionPhase::WaitingForBell ||
		!LabState->ApplyRequest(Request))
	{
		return false;
	}

	if (SetSessionPhase(ECPLabSessionPhase::Request))
	{
		return true;
	}

	LabState->ClearRequest();
	return false;

}

bool ACPLabGameMode::TryAcceptLabRequest()
{
	ACPLabGameState* LabState = GetLabGameState();
	if (!LabState ||
		LabState->GetCurrentPhase() != ECPLabSessionPhase::Request ||
		!LabState->HasActiveRequest())
	{
		return false;
	}

	return SetSessionPhase(ECPLabSessionPhase::Preparing);
}


void ACPLabGameMode::DebugAdvanceSessionPhase()
{
	const ACPLabGameState* LabState = GetLabGameState();
	if (!LabState)
	{
		return;
	}

	switch (LabState->GetCurrentPhase())
	{
	case ECPLabSessionPhase::WaitingForBell:
		TryStartLabSession();
		break;

	case ECPLabSessionPhase::Request:
		TryAcceptLabRequest();
		break;

	case ECPLabSessionPhase::Preparing:
		SetSessionPhase(ECPLabSessionPhase::Processing);
		break;

	case ECPLabSessionPhase::Processing:
		SetSessionPhase(ECPLabSessionPhase::Result);
		break;

	case ECPLabSessionPhase::Result:
		ResetLabSession();
		break;

	default:
		break;
	}
}


