// CPPlayerAbilitySystemComponent.cpp

#include "Character/GAS/CPPlayerAbilitySystemComponent.h"

#include "Abilities/GameplayAbility.h"
#include "GameFramework/Actor.h"

UCPPlayerAbilitySystemComponent::UCPPlayerAbilitySystemComponent() : StartupAbilityLevel(1)
{
	PrimaryComponentTick.bCanEverTick = false;
	
}

void UCPPlayerAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilitySystem();
}

void UCPPlayerAbilitySystemComponent::InitializeAbilitySystem()
{
	AActor* OwningActor = GetOwner();

	if (!IsValid(OwningActor))
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerASC] OwningActor가 유효하지 않습니다."));
		return;
	}

	InitAbilityActorInfo(OwningActor, OwningActor);

	UE_LOG(LogTemp, Log, TEXT("[PlayerASC] ActorInfo 초기화 완료: %s"), *GetNameSafe(OwningActor));

	if (OwningActor->HasAuthority())
	{
		GrantStartupAbilities();
	}
}

void UCPPlayerAbilitySystemComponent::GrantStartupAbilities()
{
	AActor* OwningActor = GetOwner();

	if (!IsValid(OwningActor) || !OwningActor->HasAuthority())
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartupAbilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		if (FindAbilitySpecFromClass(AbilityClass))
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(
			AbilityClass,
			StartupAbilityLevel,
			INDEX_NONE,
			OwningActor
		);

		GiveAbility(AbilitySpec);

		UE_LOG(LogTemp, Log, TEXT("[PlayerASC] Startup Ability 부여: %s"), *GetNameSafe(AbilityClass));
	}
}

