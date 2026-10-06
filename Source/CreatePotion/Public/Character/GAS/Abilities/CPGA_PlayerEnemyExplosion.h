#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CPGA_PlayerEnemyExplosion.generated.h"

class ACharacter;
class UBrainComponent;
class UCPHealthComponent;
class UCPInteractionComponent;
class USkeletalMeshComponent;
class USoundBase;

/** 포션 중심으로 래그돌을 모은 뒤 방출하고 Player/Enemy 상태를 복구한다. */
UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_PlayerEnemyExplosion : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCPGA_PlayerEnemyExplosion();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// NPC Explosion BP와 같은 흡입·방출 설정
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Pull", meta = (ClampMin = "0"))
	float PullDelay = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Pull")
	float CenterZOffset = 100.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Pull", meta = (ClampMin = "0"))
	float OrbitRadius = 150.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Pull", meta = (ClampMin = "0", ClampMax = "0.5"))
	float OrbitRadiusJitter = 0.2f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Pull")
	float OrbitSpeedDegPerSec = 1000.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Pull", meta = (ClampMin = "0.05"))
	float PullDuration = 2.3f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Pull", meta = (ClampMin = "0"))
	float PullStiffness = 12.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Pull", meta = (ClampMin = "0"))
	float MaxPullSpeed = 5000.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Pull", meta = (ClampMin = "0"))
	float WindFollowGain = 10.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Pull", meta = (ClampMin = "0", ClampMax = "1"))
	float CoreDriveRatio = 0.6f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Pull", meta = (ClampMin = "0"))
	float MaxPullAccel = 6000.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Pull")
	float TurbulenceSpin = 200.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Blast", meta = (ClampMin = "0"))
	float ExplosionSpeed = 5000.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Blast")
	float ExplosionUpwardBias = 0.6f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Blast")
	float ExplosionSpin = 300.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Blast", meta = (ClampMin = "0", ClampMax = "1"))
	float LimbSpeedRatio = 0.6f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Blast", meta = (ClampMin = "0"))
	float BounceDuration = 0.6f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Blast", meta = (ClampMin = "0"))
	float BounceMinSpeed = 250.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Blast")
	float BounceSpin = 100.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Recovery", meta = (ClampMin = "0.1"))
	float MaxSettleTime = 8.0f;
	// 폭발 순간 한 번만 Player 체력에서 차감할 값
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Damage", meta = (ClampMin = "0"))
	float PlayerDamage = 2.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Cue")
	FGameplayTag GatherCueTag;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion|Cue")
	FGameplayTag ExplodeCueTag;
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> StartSFX;
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> EndSFX;

private:
	// 하나의 타이머에서 대기→흡입→방출→정착 단계 진행
	enum class EPhase : uint8 { Waiting, Pulling, Blasted };
	void UpdateReaction();
	void ApplyPull(float DeltaTime);
	void Blast();
	void FinishReaction();
	void GetBodyState(FVector& Position, FVector& Velocity, float& Mass) const;
	USkeletalMeshComponent* FindVisualMesh(ACharacter* Character) const;
	UFUNCTION()
	void HandleDeath(AActor* DamageCauser);
	UFUNCTION()
	void HandleEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);
	UFUNCTION()
	void HandleMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	TWeakObjectPtr<ACharacter> CachedCharacter;
	TWeakObjectPtr<USkeletalMeshComponent> CachedMesh;
	TWeakObjectPtr<USceneComponent> OriginalParent;
	TWeakObjectPtr<UBrainComponent> CachedBrain;
	TWeakObjectPtr<AController> CachedController;
	TWeakObjectPtr<UCPHealthComponent> CachedHealth;
	TWeakObjectPtr<UCPInteractionComponent> CachedInteraction;
	TWeakObjectPtr<AActor> DamageInstigator;
	FTransform OriginalRelativeTransform;
	FVector OriginalActorLocation;
	FVector Center;
	FName OriginalSocket;
	FName OriginalMeshProfile;
	FName CoreBone;
	ECollisionEnabled::Type OriginalMeshCollision;
	ECollisionEnabled::Type OriginalCapsuleCollision;
	EMovementMode OriginalMovementMode;
	uint8 OriginalCustomMovementMode = 0;
	bool bOriginalGravity = false;
	bool bOriginalCCD = false;
	bool bOriginalNotifyHit = false;
	bool bOriginalInteractionBlocked = false;
	bool bStartedAsSlime = false;
	bool bPausedBrain = false;
	bool bBlockedInput = false;
	bool bPrepared = false;
	EPhase Phase = EPhase::Waiting;
	double PhaseStartTime = 0.0;
	double LastTickTime = 0.0;
	double LastBounceTime = -1.0;
	float StartRadius = 0.0f;
	float ActualOrbitRadius = 0.0f;
	float TurbulenceElapsed = 0.0f;
	FDelegateHandle DyingHandle;
	FTimerHandle ReactionTimer;
};
