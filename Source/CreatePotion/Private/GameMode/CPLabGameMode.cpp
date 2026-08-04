// CPLabGameMode.cpp

#include "GameMode/CPLabGameMode.h"

#include "GameState/CPLabGameState.h"
#include "PlayerState/CPLabPlayerState.h"
#include "UI/CPLabHUD.h"

ACPLabGameMode::ACPLabGameMode()
{
	GameStateClass = ACPLabGameState::StaticClass();
	PlayerStateClass = ACPLabPlayerState::StaticClass();
	HUDClass = ACPLabHUD::StaticClass();
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
	SetSessionPhase(ECPLabSessionPhase::WaitingForBell);
}

bool ACPLabGameMode::TryStartLabSession()
{
	ACPLabGameState* LabState = GetLabGameState();

	if (!LabState || LabState->GetCurrentPhase() != ECPLabSessionPhase::WaitingForBell)
	{
		return false;
	}

	return SetSessionPhase(ECPLabSessionPhase::Request);
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
		SetSessionPhase(ECPLabSessionPhase::Request);
		break;

	case ECPLabSessionPhase::Request:
		SetSessionPhase(ECPLabSessionPhase::Preparing);
		break;

	case ECPLabSessionPhase::Preparing:
		SetSessionPhase(ECPLabSessionPhase::Processing);
		break;

	case ECPLabSessionPhase::Processing:
		SetSessionPhase(ECPLabSessionPhase::Result);
		break;

	case ECPLabSessionPhase::Result:
		SetSessionPhase(ECPLabSessionPhase::WaitingForBell);
		break;

	default:
		break;
	}
}

