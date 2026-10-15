#include "Character/GAS/Abilities/GA_CPPlayerFartLaunch.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

UGA_CPPlayerFartLaunch::UGA_CPPlayerFartLaunch()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer InitialAssetTags;
	InitialAssetTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Reaction.Potion"))));
	SetAssetTags(InitialAssetTags);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Immunity.Potion.All"))));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Immunity.Potion.FartLaunch"))));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("State.Reaction.Potion"))));
}

void UGA_CPPlayerFartLaunch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* PlayerCharacter = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UWorld* World = GetWorld();
	if (!IsValid(PlayerCharacter) || !World || !HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (StartSFX) UGameplayStatics::PlaySoundAtLocation(this, StartSFX, Avatar->GetActorLocation());
	}
	bEffectSFXStarted = true;
	if (FlightSFX) FlightAudioComponent = UGameplayStatics::SpawnSoundAttached(FlightSFX, PlayerCharacter->GetRootComponent(), NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true);

	if (FartLaunchCueTag.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->AddGameplayCue(FartLaunchCueTag);
	}

	PlayerCharacter->AddActorWorldOffset(FVector(0.0f, 0.0f, 50.0f), false);

	const FVector LaunchDirection = PlayerCharacter->GetActorTransform().TransformVectorNoScale(LaunchRotation.Vector()).GetSafeNormal();
	PlayerCharacter->LaunchCharacter(LaunchDirection * LaunchForce, true, true);

	if (ReactionDuration <= 0.0f)
	{
		EndLaunchBehavior();
	}
	else
	{
		World->GetTimerManager().SetTimer(DurationTimerHandle, this, &UGA_CPPlayerFartLaunch::EndLaunchBehavior, ReactionDuration, false);
	}
}

void UGA_CPPlayerFartLaunch::EndLaunchBehavior()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_CPPlayerFartLaunch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (FlightAudioComponent.IsValid()) FlightAudioComponent->Stop();
	FlightAudioComponent.Reset();
	if (bEffectSFXStarted)
	{
		bEffectSFXStarted = false;
		if (AActor* Avatar = GetAvatarActorFromActorInfo())
		{
			if (EndSFX) UGameplayStatics::PlaySoundAtLocation(this, EndSFX, Avatar->GetActorLocation());
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DurationTimerHandle);
	}

	if (FartLaunchCueTag.IsValid() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveGameplayCue(FartLaunchCueTag);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
