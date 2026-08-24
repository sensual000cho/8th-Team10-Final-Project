#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CPGE_Giant.generated.h"

// 실제 만료는 GE의 Duration이 아니라 NPCSubsystem의 월드시간 타이머가 담당
UCLASS()
class CREATEPOTION_API UCPGE_Giant : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UCPGE_Giant();
};