#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_PotionReactionBase.h"
#include "Engine/EngineTypes.h"
#include "CPGA_FartLaunch.generated.h"

UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_FartLaunch : public UCPGA_PotionReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_FartLaunch();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	void StartRagdoll();
	void EndFloatBehavior();
	void ForceEndAbility();
	void CheckRagdollVelocity();
	void ApplyRagdollThrust();

	UFUNCTION()
	void OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float LaunchForce = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	FRotator LaunchRotation = FRotator(45.f, 0.f, 0.f);

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float RagdollDelay = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float FloatDuration = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float StopVelocityThreshold = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float MaxFailsafeTime = 8.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float ErraticSpinForce = 50000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	FGameplayTag FartLaunchCueTag;

private:
	FTimerHandle RagdollTimerHandle;
	FTimerHandle DurationTimerHandle;
	FTimerHandle VelocityCheckTimerHandle;
	FTimerHandle ThrustTimerHandle;
	FTimerHandle FailsafeTimerHandle;

	bool bIsRagdolling = false;
	float LastMeshHitTime = 0.f;
	float OriginalGravityScale = 1.0f;

	FName OriginalCapsuleCollisionProfile;
	ECollisionEnabled::Type OriginalCapsuleCollisionEnabled = ECollisionEnabled::QueryAndPhysics;
	FName OriginalMeshCollisionProfile;
	ECollisionEnabled::Type OriginalMeshCollisionEnabled = ECollisionEnabled::QueryOnly;
};