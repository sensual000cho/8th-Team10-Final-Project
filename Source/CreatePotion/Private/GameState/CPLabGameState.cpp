#include "GameState/CPLabGameState.h"

#include "Lab/Component/CPLabPotionSessionComponent.h"

ACPLabGameState::ACPLabGameState()
{
	// 모든 Lab Actor가 같은 세션 상태를 사용하도록 GameState에 생성
	PotionSession =
		CreateDefaultSubobject<UCPLabPotionSessionComponent>(
			TEXT("PotionSession"));
}

UCPLabPotionSessionComponent*
ACPLabGameState::GetPotionSession() const
{
	return PotionSession;
}
