// CPHealthComponent.cpp


#include "Character/CPHealthComponent.h"


UCPHealthComponent::UCPHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	MaxHealth = FMath::Max(MaxHealth, 1.f);
	CurrentHealth = MaxHealth;

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

float UCPHealthComponent::ApplyDamage(float DamageAmount, AActor* DamageCauser)
{
	// 비정상적인 피해량 혹은 사망한 경우
	if (!FMath::IsFinite(DamageAmount) || DamageAmount <= 0.f || IsDead())
	{
		return 0.f;
	}

	const float PreviousHealth = CurrentHealth;

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);

	const float AppliedDamage = PreviousHealth - CurrentHealth;

	UE_LOG(LogTemp, Log, TEXT("[Health] %s / 피해: %.1f / 체력: %.1f → %.1f / 공격자: %s"),
		*GetNameSafe(GetOwner()), AppliedDamage, PreviousHealth, CurrentHealth, *GetNameSafe(DamageCauser));

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	
	if (IsDead())
	{
		OnDeath.Broadcast(DamageCauser);
	}
	else if (AppliedDamage > 0.f)
	{
		OnDamaged.Broadcast(AppliedDamage, DamageCauser);
	}

	return AppliedDamage;
}

