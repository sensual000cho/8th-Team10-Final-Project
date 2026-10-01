#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_PotionReactionBase.h"
#include "Engine/EngineTypes.h"
#include "CPGA_ImpulseReactionBase.generated.h"

class ACPBaseNPC;
class USkeletalMeshComponent;
class UPrimitiveComponent;

UCLASS(Abstract, Blueprintable)
class CREATEPOTION_API UCPGA_ImpulseReactionBase : public UCPGA_PotionReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_ImpulseReactionBase();

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	virtual ACPBaseNPC* PrepareReactionNPC(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);
	virtual void EnterRagdoll(ACPBaseNPC* TargetCharacter);
	void StartVelocityCheckAndFailsafe(float StopThreshold, float FailsafeTime);
	void CheckRagdollVelocity();
	void ForceEndAbility();
	virtual void CleanupNPCReaction(const FGameplayAbilityActorInfo* ActorInfo);

	void TransitionToNaturalSettling(FTimerHandle& ActiveForceTimerHandle);

	USkeletalMeshComponent* GetRagdollingMesh() const;

	void ClearTimers(const TArray<FTimerHandle*>& Handles);
	void ProcessMeshBounce(USkeletalMeshComponent* Mesh, const FHitResult& Hit, float MinBounceSpeed, float SpinForceMultiplier = 0.0f);

	UFUNCTION()
	virtual void OnRagdollMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual float GetMeshBounceMinSpeed() const { return 0.0f; }
	virtual float GetMeshBounceSpinMultiplier() const { return 0.0f; }

protected:
	bool bIsRagdolling = false;
	bool bReactToMeshBounce = false;
	float OriginalGravityScale = 1.0f;
	float LastMeshHitTime = 0.0f;
	float StopVelocityThreshold = 20.0f;

	static constexpr float MeshHitDebounceInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Ragdoll")
	float MaxFailsafeTime = 8.0f;

	FName OriginalCapsuleCollisionProfile;
	ECollisionEnabled::Type OriginalCapsuleCollisionEnabled = ECollisionEnabled::QueryAndPhysics;
	FName OriginalMeshCollisionProfile;
	ECollisionEnabled::Type OriginalMeshCollisionEnabled = ECollisionEnabled::QueryOnly;

	FTimerHandle VelocityCheckTimerHandle;
	FTimerHandle FailsafeTimerHandle;
};