#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_CPPlayerFartLaunch.generated.h"

class USoundBase;
class UAudioComponent;

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
	TWeakObjectPtr<UAudioComponent> FlightAudioComponent;

	// 시작에 성공한 실행만 종료음을 한 번 재생
	bool bEffectSFXStarted = false;

	UFUNCTION()
	void EndLaunchBehavior();

protected:
	// 비행 중 대상을 따라 재생하며 종료·취소 시 정지하는 소리
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> FlightSFX;

	// 효과 적용 성공 시 재생할 단발음. 미지정 시 생략
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> StartSFX;

	// 효과 종료 시 재생할 단발음. 미지정 시 생략
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> EndSFX;

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
