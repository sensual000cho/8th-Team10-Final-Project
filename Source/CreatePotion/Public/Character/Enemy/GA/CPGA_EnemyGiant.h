#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CPGA_EnemyGiant.generated.h"

class ACPEnemyCharacter;

/** 분열 크기를 유지하면서 일정 시간 Enemy의 크기와 전투 배율을 높인다. */
UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_EnemyGiant : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCPGA_EnemyGiant();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 같은 효과를 다시 맞아도 이 시간을 갱신하지 않는다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Giant", meta = (ClampMin = "0.1", Units = "s"))
	float DurationSeconds = 30.0f;

	// Mesh, 충돌체, 공격 거리에 적용하는 배율이다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Giant", meta = (ClampMin = "1.0"))
	float ScaleMultiplier = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Giant", meta = (ClampMin = "1.0"))
	float WalkSpeedMultiplier = 1.5f;

	// 피해량은 유지하고 Player에게 전달하는 피격 Impulse만 늘린다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Giant", meta = (ClampMin = "1.0"))
	float AttackImpulseMultiplier = 2.0f;

private:
	void HandleDurationExpired();

	TWeakObjectPtr<ACPEnemyCharacter> CachedEnemy;
	FTimerHandle DurationTimerHandle;
	FDelegateHandle DyingDelegateHandle;
	bool bGiantApplied = false;
};
