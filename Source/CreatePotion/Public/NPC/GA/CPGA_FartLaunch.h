#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_PotionReactionBase.h"
#include "CPGA_FartLaunch.generated.h"

UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_FartLaunch : public UCPGA_PotionReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_FartLaunch();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	// 공중에 떠있는 시간이 끝나면 추진력을 멈추고 바닥에 멈출 때까지 속도를 체크
	UFUNCTION()
	void EndFloatBehavior();

	// 발사된 후 캡슐이 어딘가에 부딪히면 래그돌(물리) 상태로 전환
	UFUNCTION()
	void OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 래그돌 상태에서 메시가 바닥이나 벽에 부딪힐 때마다 불규칙하게 튕겨오르게
	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 래그돌이 바닥에 굴러가다 완전히 멈췄는지 주기적으로 확인
	UFUNCTION()
	void CheckRagdollVelocity();
	void RecoverFromRagdoll();

	UFUNCTION()
	void ApplyRagdollThrust();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	float FloatDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	float LaunchForce = 3000.f; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	float ErraticSpinForce = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	FRotator LaunchRotation = FRotator(0.f, 45.f, 0.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	float StopVelocityThreshold = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	FGameplayTag FartLaunchCueTag;

private:
	float OriginalGravityScale = 1.0f;
	float LastMeshHitTime = 0.f;
	bool bIsRagdolling = false;

	FTimerHandle DurationTimerHandle;
	FTimerHandle VelocityCheckTimerHandle;
	FTimerHandle ThrustTimerHandle;
};