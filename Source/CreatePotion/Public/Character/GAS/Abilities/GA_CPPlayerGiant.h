#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Core/CameraRigInstanceID.h"
#include "TimerManager.h"
#include "GA_CPPlayerGiant.generated.h"

class ACharacter;
class UCapsuleComponent;
class UCPCarryComponent;
class UCameraRigAsset;
class UGameplayCameraComponentBase;
class USkeletalMeshComponent;

/**
 * Player-specific implementation of Potion.Effect.Giant.
 * Owns its Player presentation, duration, and restoration behavior without depending on the NPC GA hierarchy.
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant", meta = (ClampMin = "0.0", Units = "s"))
	float DurationSeconds = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant", meta = (ClampMin = "0.0"))
	float Magnitude = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant", meta = (ClampMin = "0.0"))
	float ThrowSpeedMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant|Camera")
	TObjectPtr<UCameraRigAsset> GiantCameraRig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant|Camera")
	FVector GiantCameraTranslationOffset = FVector(-600.0f, 0.0f, 150.0f);

private:
	UFUNCTION()
	void HandleDurationExpired();

	void CacheOriginalState(ACharacter* PlayerCharacter);
	void ApplyGiantState();
	void RestoreOriginalState();
	void ApplyGiantCamera();
	void RestoreGiantCamera();

	TWeakObjectPtr<ACharacter> CachedCharacter;
	TWeakObjectPtr<USkeletalMeshComponent> CachedMesh;
	TWeakObjectPtr<UCapsuleComponent> CachedCapsule;
	TWeakObjectPtr<UCPCarryComponent> CachedCarryComponent;
	TWeakObjectPtr<UGameplayCameraComponentBase> CachedGameplayCamera;
	FCameraRigInstanceID GiantCameraRigInstance;
	FTimerHandle DurationTimerHandle;

	FVector OriginalMeshScale = FVector::OneVector;
	FVector OriginalMeshLocation = FVector::ZeroVector;
	FVector OriginalCarryLocation = FVector::ZeroVector;
	float OriginalCapsuleRadius = 0.0f;
	float OriginalCapsuleHalfHeight = 0.0f;
	float OriginalThrowSpeedMultiplier = 1.0f;
	bool bGiantApplied = false;
};
