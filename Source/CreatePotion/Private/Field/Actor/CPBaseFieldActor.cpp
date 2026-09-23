#include "Field/Actor/CPBaseFieldActor.h"
#include "AbilitySystemComponent.h"

ACPBaseFieldActor::ACPBaseFieldActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

UAbilitySystemComponent* ACPBaseFieldActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool ACPBaseFieldActor::PotionReaction(const FGameplayTag& EventTag, AActor* InstigatorActor)
{
	if (!AbilitySystemComponent) return false;
	
	FGameplayEventData EventData;
	
	EventData.EventTag = EventTag;
	EventData.Instigator = InstigatorActor;
	EventData.Target = this;
	
	const int32 ActivatedAbilityCount = AbilitySystemComponent->HandleGameplayEvent(EventTag, &EventData);
	
	return ActivatedAbilityCount > 0;
}

void ACPBaseFieldActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (!AbilitySystemComponent) return;
	
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	
	GrantPotionReactionAbilities();
}

void ACPBaseFieldActor::GrantPotionReactionAbilities()
{
	if (!AbilitySystemComponent) return;
	
	for (const TSubclassOf<UGameplayAbility>& Abilityclass : PotionReactionAbilities)
	{
		if (!Abilityclass) continue;
		
		FGameplayAbilitySpec AbilitySpec(Abilityclass, 1, INDEX_NONE, this);
		
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}