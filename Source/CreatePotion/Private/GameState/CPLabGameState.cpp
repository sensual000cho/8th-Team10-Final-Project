// CPLabGameState.cpp

#include "GameState/CPLabGameState.h"

bool ACPLabGameState::ApplySessionPhase(ECPLabSessionPhase NewPhase)
{
	if (CurrentPhase == NewPhase)
	{
		return false;
	}

	CurrentPhase = NewPhase;
	//갱 신 후 CurrentPhase 뭔지 Broadcast 하기
	OnSessionPhaseChanged.Broadcast(CurrentPhase);

	UE_LOG(LogTemp, Log, TEXT("[Lab] Session Phase -> %s"), *UEnum::GetValueAsString(CurrentPhase));
	return true;
}
