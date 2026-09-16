#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_PotionReactionBase.h"
#include "CPGA_Reset.generated.h"

UCLASS()
class CREATEPOTION_API UCPGA_Reset : public UCPGA_PotionReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_Reset();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};