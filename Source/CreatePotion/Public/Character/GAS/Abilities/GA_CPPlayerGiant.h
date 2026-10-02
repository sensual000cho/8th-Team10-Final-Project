#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Core/CameraRigInstanceID.h"
#include "GA_CPPlayerGiant.generated.h"

class USoundBase;
class UNiagaraSystem;
class UNiagaraComponent;

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
 * 외형, 캡슐, 이동 속도와 카메라의 거대화 적용 및 복구를 관리한다.
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
	// 거대화 시작 연출. NPC와 같은 Niagara 에셋을 BP에서 지정
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Giant|Visual")
	TObjectPtr<UNiagaraSystem> GiantVFX;

	UPROPERTY(EditDefaultsOnly, Category = "Potion|Giant|Visual", meta = (ClampMin = "0"))
	float GiantVFXScale = 2.5f;

	// 효과 적용 성공 시 재생할 단발음. 미지정 시 생략
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> StartSFX;

	// 효과 종료 시 재생할 단발음. 미지정 시 생략
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> EndSFX;

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
	// 중화·종료 시 아직 남은 시작 연출 제거
	TWeakObjectPtr<UNiagaraComponent> GiantVFXComponent;

	// 시작에 성공한 실행만 종료음을 한 번 재생
	bool bEffectSFXStarted = false;

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
	// 현재 이동 계산의 Giant 속도 배율 적용 여부
	bool bSpeedApplied = false;
	bool bGiantApplied = false;
};
