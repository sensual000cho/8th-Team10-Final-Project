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
	if (!CanReactToTag(EventTag)) return false;
	
	FGameplayEventData EventData;
	
	EventData.EventTag = EventTag;
	EventData.Instigator = InstigatorActor;
	EventData.Target = this;
	
	const int32 ActivatedAbilityCount = AbilitySystemComponent->HandleGameplayEvent(EventTag, &EventData);
	
	UE_LOG(
		LogTemp,
		Error,
		TEXT("Send Potion Event | Target=%s | Tag=%s"),
		*GetNameSafe(this),
		*EventData.EventTag.ToString()
	);
	
	return ActivatedAbilityCount > 0;
}

bool ACPBaseFieldActor::CanReactToTag(const FGameplayTag& EventTag) const
{
	return EventTag.IsValid() && ReactionEventTags.HasTagExact(EventTag);
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