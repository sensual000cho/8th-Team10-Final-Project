#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CPGA_PlayerEnemyGust.generated.h"

class ACharacter;
class UActorComponent;
class UCharacterMovementComponent;
class UCPHealthComponent;

// Player와 Enemy의 이동속도만 높이는 지속 효과.
UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_PlayerEnemyGust : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCPGA_PlayerEnemyGust();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 재적용은 무시하고 최초 적용부터 시간을 센다.
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Gust", meta = (ClampMin = "0.01", Units = "s"))
	float DurationSeconds = 30.0f;

	// NPC Gust와 같은 기본 배율. Giant 배율과 곱한다.
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Gust", meta = (ClampMin = "1.0"))
	float WalkSpeedMultiplier = 5.0f;

private:
	void HandleDurationExpired();
	UFUNCTION()
	void HandlePlayerDeath(AActor* DamageCauser);
	UFUNCTION()
	void HandlePreCharacterMovement();
	UFUNCTION()
	void RestoreCalculatedWalkSpeed(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);
	// 현재 속도에서 자신이 적용한 배율만 제거한다.
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
