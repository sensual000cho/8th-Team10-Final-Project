#include "Field/GA/GA_CPNestPotionReaction.h"
#include "Field/Actor/CPNestActor.h"

UGA_CPNestPotionReaction::UGA_CPNestPotionReaction()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FAbilityTriggerData TriggerData;
	
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Potion.Effect.FartLaunch"));
	
	AbilityTriggers.Add(TriggerData);
}

void UGA_CPNestPotionReaction::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || !TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		
		return;
	}
	
	ACPNestActor* Nest = Cast<ACPNestActor>(ActorInfo->AvatarActor.Get());
	
	if (!Nest)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		
		return;
	}
	
	Nest->PotionReaction(TriggerEventData->EventTag);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
