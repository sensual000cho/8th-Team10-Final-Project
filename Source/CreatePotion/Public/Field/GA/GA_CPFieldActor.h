#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_CPFieldActor.generated.h"

// 필드 액터 전용 공용 GA
UCLASS()
class CREATEPOTION_API UGA_CPFieldActor : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_CPFieldActor();
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reaction")
	TSubclassOf<UGameplayEffect> StateEffectClass;
};
