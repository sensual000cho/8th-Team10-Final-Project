#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CPGA_PlayerEnemyGust.generated.h"

class USoundBase;

class ACharacter;
class UActorComponent;
class UCharacterMovementComponent;
class UCPHealthComponent;

/** Player와 Enemy의 이동 속도에 Gust 배율을 적용하는 지속 효과. */
UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_PlayerEnemyGust : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCPGA_PlayerEnemyGust();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 효과 적용 성공 시 재생할 단발음. 미지정 시 생략
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> StartSFX;

	// 효과 종료 시 재생할 단발음. 미지정 시 생략
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> EndSFX;

	// Gust 유지 시간. 활성 중 재적용 시 갱신하지 않음
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Gust", meta = (ClampMin = "0.01", Units = "s"))
	float DurationSeconds = 30.0f;

	// 이동 속도 배율. 다른 효과의 속도 배율과 곱연산
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Gust", meta = (ClampMin = "1.0"))
	float WalkSpeedMultiplier = 5.0f;

private:
	// 시작에 성공한 실행만 종료음을 한 번 재생
	bool bEffectSFXStarted = false;

	void HandleDurationExpired();
	UFUNCTION()
	void HandlePlayerDeath(AActor* DamageCauser);

	// Player BP의 속도 계산 이후, CMC 이동 직전에 배율 적용
	UFUNCTION()
	void HandlePreCharacterMovement();

	// CMC 이동 이후 Gust 배율 제거
	UFUNCTION()
	void RestoreCalculatedWalkSpeed(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	// 다른 효과의 속도 배율을 보존하며 Gust 배율만 제거
	void RemoveSpeedMultiplier();

	TWeakObjectPtr<ACharacter> CachedCharacter;
	TWeakObjectPtr<UCharacterMovementComponent> CachedMovement;
	TWeakObjectPtr<UActorComponent> CachedPreCMCTick;
	TWeakObjectPtr<UCPHealthComponent> CachedHealth;
	FDelegateHandle DyingDelegateHandle;
	FTimerHandle DurationTimerHandle;
	float AppliedMultiplier = 1.0f;
	bool bSpeedApplied = false;
};
