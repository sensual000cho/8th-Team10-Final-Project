// CPHealthComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCPOnHealthChanged, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCPOnHealthDamaged, float, AppliedDamage, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCPOnHealthDeath, AActor*, DamageCauser);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TArray<TObjectPtr<class USoundBase>> HurtSounds;

	UCPHealthComponent();
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	float ApplyDamage(float DamageAmount, AActor* DamageCauser);
	
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }
	
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return CurrentHealth <= 0.f; }
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void RestoreFullHealth();
	
	// UI 체력 표시 갱신용
	UPROPERTY(BlueprintAssignable, Category = "Health|Event")
	FCPOnHealthChanged OnHealthChanged;

	// 피격 연출
	UPROPERTY(BlueprintAssignable, Category = "Health|Event")
	FCPOnHealthDamaged OnDamaged;
	
	// 사망 연출
	UPROPERTY(BlueprintAssignable, Category = "Health|Event")
	FCPOnHealthDeath OnDeath;

protected:
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Audio variation must not consume the gameplay random sequence.
	FRandomStream HurtSoundRandom;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float MaxHealth = 40.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float  CurrentHealth = 40.f;
	
	// 피격이 없으면 10초 뒤 피회복
	UPROPERTY(EditDefaultsOnly, Category = "Health|Regen", meta = (ClampMin = "0.01", Units = "s"))
	float RegenDelay = 10.f;
	// 피회복 주기(초)
	UPROPERTY(EditDefaultsOnly, Category = "Health|Regen", meta = (ClampMin = "0.01", Units = "s"))
	float RegenInterval = 1.f;
	// 회복량
	UPROPERTY(EditDefaultsOnly, Category = "Health|Regen", meta = (ClampMin = "0.0"))
	float RegenAmount = 10.f;
	
	// 피격 시 회복 대기(10초)
	void RestartHealthRegen();
	// 회복 중단
	void StopHealthRegen();
	// 체력 회복
	void RegenerateHealth();

	FTimerHandle HealthRegenTimerHandle;
};
