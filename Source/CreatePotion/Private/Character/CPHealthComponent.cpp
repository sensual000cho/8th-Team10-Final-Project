// CPHealthComponent.cpp


#include "Character/CPHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

UCPHealthComponent::UCPHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	HurtSoundRandom.Initialize(static_cast<int32>(FPlatformTime::Cycles()));
	
	MaxHealth = FMath::Max(MaxHealth, 1.f);
	CurrentHealth = MaxHealth;

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UCPHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopHealthRegen();

	Super::EndPlay(EndPlayReason);
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
	
	// 피격 시 회복 타이머 발동(죽으면X)
	if (AppliedDamage > 0.f)
	{
		if (IsDead())
		{
			StopHealthRegen();
		}
		else
		{
			RestartHealthRegen();
		}
	}
	
	if (IsDead())
	{
		OnDeath.Broadcast(DamageCauser);
	}
	else if (AppliedDamage > 0.f)
	{
		if (const APawn* Player = Cast<APawn>(GetOwner()); Player && Player->IsPlayerControlled())
		{
			if (!HurtSounds.IsEmpty())
			{
				if (USoundBase* Sound = HurtSounds[HurtSoundRandom.RandRange(0, HurtSounds.Num() - 1)]) UGameplayStatics::PlaySoundAtLocation(this, Sound, Player->GetActorLocation());
			}
		}
		OnDamaged.Broadcast(AppliedDamage, DamageCauser);
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}

	return AppliedDamage;
}

void UCPHealthComponent::RestoreFullHealth()
{
	StopHealthRegen();
	
	CurrentHealth = MaxHealth;
	
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UCPHealthComponent::RestartHealthRegen()
{
	StopHealthRegen();

	UWorld* World = GetWorld();

	if (!World || IsDead() || CurrentHealth >= MaxHealth)
	{
		return;
	}

	if (!FMath::IsFinite(RegenDelay) || RegenDelay <= 0.f ||
		!FMath::IsFinite(RegenInterval) || RegenInterval <= 0.f ||
		!FMath::IsFinite(RegenAmount) || RegenAmount <= 0.f)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		HealthRegenTimerHandle,
		this,
		&UCPHealthComponent::RegenerateHealth,
		RegenInterval, // 첫 회복 이후에는 1초마다
		true,          
		RegenDelay     // 첫 회복은 10초 뒤
	);
}

void UCPHealthComponent::StopHealthRegen()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HealthRegenTimerHandle);
	}
}

void UCPHealthComponent::RegenerateHealth()
{
	// 죽은 상태면 회복 불가능
	if (IsDead() || CurrentHealth >= MaxHealth || !FMath::IsFinite(RegenAmount) || RegenAmount <= 0.f)
	{
		StopHealthRegen();
		return;
	}

	const float PreviousHealth = CurrentHealth;

	CurrentHealth = FMath::Clamp(CurrentHealth + RegenAmount, 0.f, MaxHealth);

	// 최대 체력에 도달하면 종료
	if (CurrentHealth >= MaxHealth)
	{
		StopHealthRegen();
	}

	if (CurrentHealth > PreviousHealth)
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}
}
