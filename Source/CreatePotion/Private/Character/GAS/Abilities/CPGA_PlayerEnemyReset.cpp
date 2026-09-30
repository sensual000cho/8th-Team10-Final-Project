#include "Character/GAS/Abilities/CPGA_PlayerEnemyReset.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

UCPGA_PlayerEnemyReset::UCPGA_PlayerEnemyReset()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Reaction.Potion")));
	SetAssetTags(AssetTags);

	ResettableEffectTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Effect")));
	ExcludedEffectTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Respawning")));

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Potion.Effect.Reset"));
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

bool UCPGA_PlayerEnemyReset::IsResettable(const FGameplayTagContainer& EffectTags) const
{
	return EffectTags.HasAny(ResettableEffectTags) && !EffectTags.HasAny(ExcludedEffectTags);
}

void UCPGA_PlayerEnemyReset::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (StartSFX) UGameplayStatics::PlaySoundAtLocation(this, StartSFX, Avatar->GetActorLocation());
	}

	// 종료 콜백이 Ability 목록을 바꿀 수 있으므로 먼저 대상을 모은다.
	TArray<FGameplayAbilitySpecHandle> AbilitiesToCancel;
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Handle != Handle && Spec.IsActive() && Spec.Ability && IsResettable(Spec.Ability->GetAssetTags()))
		{
			AbilitiesToCancel.Add(Spec.Handle);
		}
	}

	TArray<FActiveGameplayEffectHandle> EffectsToRemove;
	for (const FActiveGameplayEffectHandle EffectHandle : ASC->GetActiveEffects(FGameplayEffectQuery()))
	{
		const FActiveGameplayEffect* Effect = ASC->GetActiveGameplayEffect(EffectHandle);
		if (!Effect) continue;
		FGameplayTagContainer EffectTags;
		Effect->Spec.GetAllAssetTags(EffectTags);
		Effect->Spec.GetAllGrantedTags(EffectTags);
		if (IsResettable(EffectTags)) EffectsToRemove.Add(EffectHandle);
	}

	// 원상복구는 각 GA의 EndAbility와 GE 제거 처리에 맡긴다. Loose Tag는 건드리지 않는다.
	for (const FGameplayAbilitySpecHandle AbilityHandle : AbilitiesToCancel) ASC->CancelAbilityHandle(AbilityHandle);
	for (const FActiveGameplayEffectHandle EffectHandle : EffectsToRemove) ASC->RemoveActiveGameplayEffect(EffectHandle);
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (EndSFX) UGameplayStatics::PlaySoundAtLocation(this, EndSFX, Avatar->GetActorLocation());
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
