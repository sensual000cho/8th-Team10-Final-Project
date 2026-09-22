#include "NPC/GA/CPGA_Reset.h"
#include "NPC/CPBaseNPC.h"
#include "AbilitySystemComponent.h"
#include "GameInstance/Subsystem/CPNPCSubsystem.h"
#include "Engine/Engine.h"

UCPGA_Reset::UCPGA_Reset()
{
	ImmunityTag = FGameplayTag::RequestGameplayTag(FName("Immunity.Potion.Reset"));
}

void UCPGA_Reset::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACPBaseNPC* TargetNPC = GetValidatedOwningPotionNPC(Handle, ActorInfo, ActivationInfo);
	if (!TargetNPC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;

	if (ASC)
	{
		for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (Spec.IsActive() && Spec.Handle != Handle)
			{
				ASC->CancelAbilityHandle(Spec.Handle);
			}
		}
	}

	if (UCPNPCSubsystem* Subsystem = TargetNPC->GetNPCSubsystem())
	{
		const FName NPCId = TargetNPC->GetPotionNPCId();
		FCPNPCEffectSaveData SaveData;
		if (Subsystem->GetNPCEffectData(NPCId, SaveData))
		{
			TArray<FGameplayTag> ActiveEffectTags;
			SaveData.ActiveEffects.GetKeys(ActiveEffectTags);

			for (const FGameplayTag& EffectTag : ActiveEffectTags)
			{
				Subsystem->ClearNPCEffect(NPCId, EffectTag);
			}
		}
	}

	if (ASC)
	{
		FGameplayTagContainer CurrentOwnedTags;
		ASC->GetOwnedGameplayTags(CurrentOwnedTags);

		const FGameplayTag StateRootTag = FGameplayTag::RequestGameplayTag(FName("State"));
		const FGameplayTag ResetTag = FGameplayTag::RequestGameplayTag(FName("State.Reaction.Potion"));

		for (const FGameplayTag& Tag : CurrentOwnedTags)
		{
			if (Tag.MatchesTag(StateRootTag) && !Tag.MatchesTagExact(ResetTag))
			{
				const int32 Count = ASC->GetTagCount(Tag);
				if (Count > 0)
				{
					ASC->RemoveLooseGameplayTag(Tag, Count);
				}
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}