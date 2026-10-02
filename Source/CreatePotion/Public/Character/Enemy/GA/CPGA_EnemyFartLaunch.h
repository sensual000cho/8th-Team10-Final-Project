#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CPGA_EnemyFartLaunch.generated.h"

class USoundBase;
class UAudioComponent;

class ACPEnemyCharacter;
class UAnimInstance;
class UAnimSequence;

/**
 * Potion.Effect.FartLaunch의 Enemy 전용 구현.
 * 한 번 발사한 뒤 CharacterMovement의 중력으로 비행하며 착지하면 사망한다.
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
	// 비행 중 대상을 따라 재생하며 종료·취소 시 정지하는 소리
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> FlightSFX;

	// 효과 적용 성공 시 재생할 단발음. 미지정 시 생략
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> StartSFX;

	// 효과 종료 시 재생할 단발음. 미지정 시 생략
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> EndSFX;

	// 날아가는 동안 반복 재생할 Animation. BP GA에서 지정한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|FartLaunch|Animation")
	TObjectPtr<UAnimSequence> LaunchAnimation;

	// 폭발 순간 부여하는 초기 속도다. 이후의 상승/낙하는 중력에 맡긴다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|FartLaunch|Launch", meta = (ClampMin = "0.0", Units = "cm/s"))
	float LaunchForce = 2000.0f;

	// 수직 상향에서 벗어날 수 있는 최대 발사 각도다. 발사마다 한 번 선택한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|FartLaunch|Launch", meta = (ClampMin = "0.0", ClampMax = "89.0", Units = "deg"))
	float LaunchConeHalfAngle = 15.0f;

	// 발사 시 정한 무작위 축을 중심으로 일정하게 회전한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|FartLaunch|Animation", meta = (ClampMin = "0.0", Units = "deg/s"))
	float SpinSpeed = 180.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|FartLaunch")
	FGameplayTag FartLaunchCueTag;

private:
	TWeakObjectPtr<UAudioComponent> FlightAudioComponent;

	// 시작에 성공한 실행만 종료음을 한 번 재생
	bool bEffectSFXStarted = false;

	UFUNCTION()
	void HandleLanded(const FHitResult& Hit);

	UFUNCTION()
	void UpdateFlightSpin(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	void ApplyLandingDamage();
	void RestoreLivingState();

	TWeakObjectPtr<ACPEnemyCharacter> CachedEnemy;
	TWeakObjectPtr<AActor> CachedDamageCauser;

	TSubclassOf<UAnimInstance> OriginalAnimInstanceClass;
	FRotator OriginalMeshRelativeRotation = FRotator::ZeroRotator;
	// 비행 중에는 회전축을 다시 뽑지 않는다.
	FVector SpinAxis = FVector::UpVector;
	float SpinAngle = 0.0f;
	EMovementMode OriginalMovementMode = MOVE_Walking;
	uint8 OriginalCustomMovementMode = 0;
	bool bLandingDamageApplied = false;
	bool bLaunchApplied = false;
};
