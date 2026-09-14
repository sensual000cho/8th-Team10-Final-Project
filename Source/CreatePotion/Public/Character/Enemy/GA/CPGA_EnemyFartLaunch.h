#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CPGA_EnemyFartLaunch.generated.h"

class ACPEnemyCharacter;
class UAnimInstance;
class UAnimSequence;

/**
 * Potion.Effect.FartLaunch의 Enemy 전용 구현.
 * Mesh Ragdoll 없이 위로 상승한 뒤 착지하면 사망한다.
 */
UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_EnemyFartLaunch : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCPGA_EnemyFartLaunch();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 날아가는 동안 반복 재생할 Animation. BP GA에서 지정한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|FartLaunch|Animation")
	TObjectPtr<UAnimSequence> LaunchAnimation;

	// 첫 바닥 충돌부터 유지할 사망 Pose. BP GA에서 지정한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|FartLaunch|Animation")
	TObjectPtr<UAnimSequence> DeathPoseAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|FartLaunch|Launch", meta = (ClampMin = "0.0"))
	float LaunchForce = 2000.0f;

	// 이 시간 동안 중력 없이 위로 상승한 뒤 낙하한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|FartLaunch|Launch", meta = (ClampMin = "0.0", Units = "s"))
	float RiseDuration = 1.5f;

	// 이동 속도에는 영향을 주지 않고 Mesh만 비틀거리게 한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|FartLaunch|Animation", meta = (ClampMin = "0.0", Units = "deg"))
	float TumbleDegreesPerStep = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|FartLaunch")
	FGameplayTag FartLaunchCueTag;

private:
	UFUNCTION()
	void HandleLanded(const FHitResult& Hit);

	void BeginFall();
	void CommitLandingDeath(const FHitResult& Hit, const FVector& ImpactVelocity);
	void StartDeathRoll(const FHitResult& Hit, const FVector& ImpactVelocity);
	void ApplyVisualTumble();
	void RestoreLivingState();

	TWeakObjectPtr<ACPEnemyCharacter> CachedEnemy;
	TWeakObjectPtr<AActor> CachedDamageCauser;

	TSubclassOf<UAnimInstance> OriginalAnimInstanceClass;
	FRotator OriginalMeshRelativeRotation = FRotator::ZeroRotator;
	EMovementMode OriginalMovementMode = MOVE_Walking;
	uint8 OriginalCustomMovementMode = 0;
	float OriginalGravityScale = 1.0f;
	float OriginalFallingLateralFriction = 0.0f;
	FTimerHandle RiseTimerHandle;
	FTimerHandle VisualTumbleTimerHandle;
	bool bLandingCommitted = false;
	bool bLaunchApplied = false;
};
