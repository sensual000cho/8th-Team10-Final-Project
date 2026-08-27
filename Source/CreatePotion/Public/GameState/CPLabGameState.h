#pragma once

#include "CoreMinimal.h"
#include "GameState/CPGameStateBase.h"
#include "CPLabGameState.generated.h"

class UCPLabPotionSessionComponent;

// 레벨이 유지되는 동안 포션 세션 컴포넌트를 소유
UCLASS()
class CREATEPOTION_API ACPLabGameState : public ACPGameStateBase
{
	GENERATED_BODY()
};
