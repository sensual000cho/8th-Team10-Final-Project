#include "Field/GA/GA_CPFieldActor.h"
#include "AbilitySystemComponent.h"
#include "Field/Actor/CPBaseFieldActor.h"

UGA_CPFieldActor::UGA_CPFieldActor()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_CPFieldActor::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || !TriggerEventData || !StateEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		
		return;
	}
	
	ACPBaseFieldActor* FieldActor = Cast<ACPBaseFieldActor>(ActorInfo->AvatarActor.Get());
	if (!FieldActor || !FieldActor->CanReactToTag(TriggerEventData->EventTag))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		
		return;
	}
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		
		return;
	}
	
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	
	EffectContext.AddSourceObject(this);
	
	FGameplayEffectSpecHandle EffectSpec =
		ASC->MakeOutgoingSpec(StateEffectClass, GetAbilityLevel(Handle, ActorInfo), EffectContext);
	
	if (!EffectSpec.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		
		return;
	}
	
	ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
