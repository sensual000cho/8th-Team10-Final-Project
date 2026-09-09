#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_CPNestPotionReaction.generated.h"

// 구역1 둥지 액터 포션 리액션 GA
UCLASS()
class CREATEPOTION_API UGA_CPNestPotionReaction : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_CPNestPotionReaction();
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
