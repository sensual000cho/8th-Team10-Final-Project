#include "NPC/GA/CPGA_PotionReactionBase.h"
#include "NPC/CPBaseNPC.h"

UCPGA_PotionReactionBase::UCPGA_PotionReactionBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer InitialAssetTags;
	InitialAssetTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Reaction.Potion")));
	SetAssetTags(InitialAssetTags);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Immunity.Potion.All")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Reaction.Potion")));
}

void UCPGA_PotionReactionBase::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (ImmunityTag.IsValid())
	{
		ActivationBlockedTags.AddTag(ImmunityTag);
	}
}

bool UCPGA_PotionReactionBase::ExtractImpactHitResult(const FGameplayEventData* TriggerEventData, FHitResult& OutHitResult)
{
	if (!TriggerEventData)
	{
		return false;
	}

	if (const FHitResult* HitResultPtr = TriggerEventData->ContextHandle.GetHitResult())
	{
		OutHitResult = *HitResultPtr;
		return true;
	}

	return false;
}

ACPBaseNPC* UCPGA_PotionReactionBase::GetOwningPotionNPC(const FGameplayAbilityActorInfo* ActorInfo) const
{
	return ActorInfo ? Cast<ACPBaseNPC>(ActorInfo->AvatarActor.Get()) : nullptr;
}

ACPBaseNPC* UCPGA_PotionReactionBase::GetValidatedOwningPotionNPC(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	ACPBaseNPC* NPC = GetOwningPotionNPC(ActorInfo);
	if (!IsValid(NPC) || !HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return nullptr;
	}
	return NPC;
}
