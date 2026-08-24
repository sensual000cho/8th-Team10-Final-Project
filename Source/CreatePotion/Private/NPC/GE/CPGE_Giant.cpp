#include "NPC/GE/CPGE_Giant.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UCPGE_Giant::UCPGE_Giant()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	UTargetTagsGameplayEffectComponent* TargetTagsComponent = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("TargetTagsComponent"));

	FInheritedTagContainer TagChanges;
	TagChanges.Added.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Effect.Giant")));
	TargetTagsComponent->SetAndApplyTargetTagChanges(TagChanges);

	GEComponents.Add(TargetTagsComponent);
}