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
	UCPHealthComponent();
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	float ApplyDamage(float DamageAmount, AActor* DamageCauser);
	
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }
	
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return CurrentHealth <= 0.f; }
	
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

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float MaxHealth = 40.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float  CurrentHealth = 40.f;
	
};
