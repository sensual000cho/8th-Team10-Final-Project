#include "NPC/GA/CPGA_PersistentReactionBase.h"
#include "NPC/CPBaseNPC.h"
#include "AbilitySystemComponent.h"

void UCPGA_PersistentReactionBase::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	CachedNPC = GetOwningPotionNPC(ActorInfo);
	if (ACPBaseNPC* NPC = CachedNPC.Get())
	{
		NPC->RegisterVisualDelegate(EffectTag, FOnPotionVisualUpdateDelegate::CreateUObject(this, &UCPGA_PersistentReactionBase::HandleApplyVisual));
	}
}

void UCPGA_PersistentReactionBase::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (ACPBaseNPC* NPC = CachedNPC.Get())
	{
		NPC->UnregisterVisualDelegate(EffectTag);
	}

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UCPGA_PersistentReactionBase::HandleApplyVisual(bool bActive, float InMagnitude)
{
	ApplyVisual(CachedNPC.Get(), bActive, InMagnitude);
}

void UCPGA_PersistentReactionBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACPBaseNPC* PotionNPC = GetValidatedOwningPotionNPC(Handle, ActorInfo, ActivationInfo);
	if (!PotionNPC) return;

	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;

	if (!ASC || !EffectTag.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ASC->HasMatchingGameplayTag(EffectTag))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	PotionNPC->RegisterPersistentPotionEffect(EffectTag, DurationWorldMinutes, Magnitude);

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}