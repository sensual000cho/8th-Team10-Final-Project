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
	// 래그돌 상태로 전환
	UFUNCTION()
	void StartRagdoll();

	// 총 시간이 지나면 추진력을 끄고 바닥으로 추락시킴
	UFUNCTION()
	void EndFloatBehavior();

	UFUNCTION()
	void OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 래그돌 상태에서 메시가 바닥이나 벽에 부딪힐 때마다 불규칙하게 튕겨오르게
	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 래그돌이 바닥에 굴러가다 완전히 멈췄는지 주기적으로 확인
	UFUNCTION()
	void CheckRagdollVelocity();

	UFUNCTION()
	void ApplyRagdollThrust();

	// 맵 밖으로 떨어지거나 너무 오래 안 멈출 때를 대비한 강제 종료
	UFUNCTION()
	void ForceEndAbility();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	float RagdollDelay = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	float FloatDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	float MaxFailsafeTime = 3.0f;

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

	FTimerHandle RagdollTimerHandle;
	FTimerHandle DurationTimerHandle;
	FTimerHandle VelocityCheckTimerHandle;
	FTimerHandle ThrustTimerHandle;
	FTimerHandle FailsafeTimerHandle;
};