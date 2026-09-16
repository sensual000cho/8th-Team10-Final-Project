#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Core/CameraRigInstanceID.h"
#include "TimerManager.h"
#include "GA_CPPlayerGiant.generated.h"

class ACharacter;
class UActorComponent;
class UCapsuleComponent;
class UCPCarryComponent;
class UCharacterMovementComponent;
class UCameraRigAsset;
class UGameplayCameraComponentBase;
class USkeletalMeshComponent;

/**
 * Potion.Effect.Giant의 Player 전용 구현.
 * NPC GA에 의존하지 않고 거대화 적용과 원상복구를 직접 관리한다.
 */
UCLASS(Blueprintable)
class CREATEPOTION_API UGA_CPPlayerGiant : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_CPPlayerGiant();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant")
	FGameplayTag EffectTag;

	// Giant 상태 유지 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant", meta = (ClampMin = "0.0", Units = "s"))
	float DurationSeconds = 30.0f;

	// Mesh와 Capsule에 적용할 크기 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant", meta = (ClampMin = "0.0"))
	float Magnitude = 3.0f;

	// Giant 상태의 투척 속도 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant", meta = (ClampMin = "0.0"))
	float ThrowSpeedMultiplier = 1.5f;

	// Character 부모 BP가 계산한 실제 이동 속도에 적용할 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant", meta = (ClampMin = "0.0"))
	float WalkSpeedMultiplier = 1.5f;

	// Giant 상태에서 Camera Modifier로 사용할 Rig
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant|Camera")
	TObjectPtr<UCameraRigAsset> GiantCameraRig;

	// Giant Camera Rig의 TranslationOffset 파라미터 값
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant|Camera")
	FVector GiantCameraTranslationOffset = FVector(-600.0f, 0.0f, 150.0f);

private:
	UFUNCTION()
	void HandleDurationExpired();

	// 부모 BP의 속도 계산 이후, CMC 이동 직전에 배율 적용
	UFUNCTION()
	void HandlePreCharacterMovement();

	// CMC 이동 이후 부모 BP가 계산한 속도로 복구
	UFUNCTION()
	void RestoreCalculatedWalkSpeed(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	void CacheOriginalState(ACharacter* PlayerCharacter);
	void ApplyGiantState();
	void RestoreOriginalState();
	// SandboxCharacter_CMC의 AC_PreCMCTick 이벤트에 연결/해제
	void BindPreCharacterMovement();
	void UnbindPreCharacterMovement();
	void ApplyGiantCamera();
	void RestoreGiantCamera();

	TWeakObjectPtr<ACharacter> CachedCharacter;
	TWeakObjectPtr<USkeletalMeshComponent> CachedMesh;
	TWeakObjectPtr<UCapsuleComponent> CachedCapsule;
	TWeakObjectPtr<UCPCarryComponent> CachedCarryComponent;
	TWeakObjectPtr<UCharacterMovementComponent> CachedCharacterMovement;
	// 부모 BP가 실제 이동 전에 호출하는 Tick 컴포넌트
	TWeakObjectPtr<UActorComponent> CachedPreCMCTickComponent;
	TWeakObjectPtr<UGameplayCameraComponentBase> CachedGameplayCamera;
	FCameraRigInstanceID GiantCameraRigInstance;
	FTimerHandle DurationTimerHandle;

	FVector OriginalMeshScale = FVector::OneVector;
	FVector OriginalMeshLocation = FVector::ZeroVector;
	FVector OriginalCarryLocation = FVector::ZeroVector;
	float OriginalCapsuleRadius = 0.0f;
	float OriginalCapsuleHalfHeight = 0.0f;
	float OriginalThrowSpeedMultiplier = 1.0f;
	// 매 프레임 부모 BP가 계산한 원본 MaxWalkSpeed
	float OriginalMaxWalkSpeed = 0.0f;
	bool bGiantApplied = false;
};
