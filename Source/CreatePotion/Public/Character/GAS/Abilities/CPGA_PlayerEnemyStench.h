#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CPGA_PlayerEnemyStench.generated.h"

class ACharacter;
class UCPHealthComponent;
class UNiagaraComponent;
class UNiagaraSystem;

// Player와 Enemy의 악취 연출과 지속 시간을 관리한다.
UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_PlayerEnemyStench : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCPGA_PlayerEnemyStench();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 활성 중 재적용은 무시한다.
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Stench", meta = (ClampMin = "0.01", Units = "s"))
	float DurationSeconds = 30.0f;

	// NPC와 같은 Niagara를 BP 참조로 지정한다.
	UPROPERTY(EditDefaultsOnly, Category = "Visual|Stench")
	TObjectPtr<UNiagaraSystem> StenchNiagaraSystem;

	// 현재 외형의 월드 크기에 곱할 연출 배율.
	UPROPERTY(EditDefaultsOnly, Category = "Visual|Stench", meta = (ClampMin = "0.01"))
	float VisualScale = 3.0f;

private:
	// 변신과 거대화 순서에 관계없이 현재 보이는 몸을 따른다.
	void UpdateVisual();
	void HandleDurationExpired();
	UFUNCTION()
	void HandlePlayerDeath(AActor* DamageCauser);
	UFUNCTION()
	void HandleOwnerEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> StenchVisual;
	TWeakObjectPtr<ACharacter> CachedCharacter;
	TWeakObjectPtr<UCPHealthComponent> CachedHealth;
	FDelegateHandle DyingDelegateHandle;
	FTimerHandle DurationTimerHandle;
	FTimerHandle VisualTimerHandle;
};
