#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_ImpulseReactionBase.h"
#include "CPGA_FartLaunch.generated.h"

UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_FartLaunch : public UCPGA_ImpulseReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_FartLaunch();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	void StartRagdoll();
	void EndFloatBehavior();
	void ApplyRagdollThrust();

	UFUNCTION()
	void OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual float GetMeshBounceMinSpeed() const override { return LaunchForce * 1.5f; }
	virtual float GetMeshBounceSpinMultiplier() const override { return ErraticSpinForce; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float LaunchForce = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	FRotator LaunchRotation = FRotator(45.f, 0.f, 0.f);

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float RagdollDelay = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float FloatDuration = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float ErraticSpinForce = 50000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	FGameplayTag FartLaunchCueTag;

private:
	FTimerHandle RagdollTimerHandle;
	FTimerHandle DurationTimerHandle;
	FTimerHandle ThrustTimerHandle;
};