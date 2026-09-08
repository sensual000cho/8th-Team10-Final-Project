#include "Field/Actor/CPBaseFieldActor.h"
#include "AbilitySystemComponent.h"

ACPBaseFieldActor::ACPBaseFieldActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

UAbilitySystemComponent* ACPBaseFieldActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ACPBaseFieldActor::PotionReaction()
{
	
}

bool ACPBaseFieldActor::CanReactToTag(const FGameplayTag& EventTag) const
{
	// 특별한 조건 없으면 true
	return true;
}

void ACPBaseFieldActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		
		if (PotionReactionAbility)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(PotionReactionAbility, 1));
		}
	}
}
