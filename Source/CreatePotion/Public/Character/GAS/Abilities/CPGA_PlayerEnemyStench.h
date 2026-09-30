#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CPGA_PlayerEnemyStench.generated.h"

class USoundBase;

class ACharacter;
class UCPHealthComponent;
class UNiagaraComponent;
class UNiagaraSystem;

/** Player와 Enemy의 악취 연출과 지속 시간을 관리한다. */
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
	// 효과 적용 성공 시 재생할 단발음. 미지정 시 생략
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> StartSFX;

	// 효과 종료 시 재생할 단발음. 미지정 시 생략
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> EndSFX;

	// Stench 유지 시간. 활성 중 재적용 시 갱신하지 않음
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Stench", meta = (ClampMin = "0.01", Units = "s"))
	float DurationSeconds = 30.0f;

	// 악취 연출에 사용할 Niagara 시스템
	UPROPERTY(EditDefaultsOnly, Category = "Visual|Stench")
	TObjectPtr<UNiagaraSystem> StenchNiagaraSystem;

	// 현재 외형의 월드 스케일에 곱할 연출 배율
	UPROPERTY(EditDefaultsOnly, Category = "Visual|Stench", meta = (ClampMin = "0.01"))
	float VisualScale = 3.0f;

private:
	// 시작에 성공한 실행만 종료음을 한 번 재생
	bool bEffectSFXStarted = false;

	// 현재 표시 중인 메시의 중심과 크기에 연출 동기화
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
