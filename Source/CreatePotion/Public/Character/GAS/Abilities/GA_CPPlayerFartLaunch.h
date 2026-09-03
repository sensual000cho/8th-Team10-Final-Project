#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_CPPlayerFartLaunch.generated.h"

/**
 * C++ contract for the player reaction to Potion.Effect.FartLaunch.
 * Blueprint children configure the event trigger, cue, tuning, and presentation.
 */
UCLASS(Blueprintable)
class CREATEPOTION_API UGA_CPPlayerFartLaunch : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_CPPlayerFartLaunch();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UFUNCTION()
	void EndLaunchBehavior();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch", meta = (ClampMin = "0.0", Units = "s"))
	float ReactionDuration = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch", meta = (ClampMin = "0.0"))
	float LaunchForce = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	FRotator LaunchRotation = FRotator(55.0f, 0.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	FGameplayTag FartLaunchCueTag;

private:
	FTimerHandle DurationTimerHandle;
};
