#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GA_CPPlayerFartLaunch.generated.h"

class ACharacter;

/**
 * Potion.Effect.FartLaunch를 받은 플레이어 전용 Reaction Ability.
 * 플레이어를 래그돌로 전환하지 않고 일정 시간 전방 위쪽으로 추진한다.
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
	void EndImpulseBehavior();

	void ApplyCharacterImpulse();

	void CacheOriginalMovementState(ACharacter* InTargetCharacter);
	void RestoreOriginalMovementState(ACharacter* InTargetCharacter);
	static const FGameplayTag& GetFartLaunchEventTag();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch", meta = (ClampMin = "0.0", Units = "s"))
	float ReactionDuration = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch", meta = (ClampMin = "0.0"))
	float LaunchForce = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	FRotator LaunchRotation = FRotator(45.0f, 0.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	FGameplayTag FartLaunchCueTag;

private:
	TWeakObjectPtr<ACharacter> TargetCharacter;

	EMovementMode OriginalMovementMode = MOVE_Walking;
	uint8 OriginalCustomMovementMode = 0;
	float OriginalFallingLateralFriction = 0.0f;
	bool bHasCachedOriginalMovementState = false;

	FTimerHandle DurationTimerHandle;
};
