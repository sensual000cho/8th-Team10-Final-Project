#include "NPC/GA/CPGA_Giant.h"
#include "NPC/CPBaseNPC.h"
#include "AbilitySystemComponent.h"

UCPGA_Giant::UCPGA_Giant()
{
	DurationWorldMinutes = 1440;
	GiantScaleMultiplier = 3.0f;
	GiantStateTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Giant"));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Immunity.Potion.Giant")));
}

void UCPGA_Giant::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;

	if (!ASC || !ActorInfo->IsNetAuthority())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ASC->HasMatchingGameplayTag(GiantStateTag))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (ACPBaseNPC* PotionNPC = GetOwningPotionNPC(ActorInfo))
	{
		PotionNPC->RegisterPersistentPotionEffect(GiantStateTag, DurationWorldMinutes, GiantScaleMultiplier);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}