#include "Character/GAS/Abilities/GA_CPPlayerFartLaunch.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	FartLaunchCueTag = FGameplayTag::RequestGameplayTag(FName(TEXT("GameplayCue.Potion.FartLaunch")));

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = GetFartLaunchEventTag();
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UGA_CPPlayerFartLaunch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* PlayerCharacter = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UCharacterMovementComponent* MovementComponent = IsValid(PlayerCharacter) ? PlayerCharacter->GetCharacterMovement() : nullptr;
	UWorld* World = GetWorld();
	const bool bHasExpectedEvent = TriggerEventData && TriggerEventData->EventTag.MatchesTagExact(GetFartLaunchEventTag());

	if (!IsValid(PlayerCharacter) || !IsValid(MovementComponent) || !World || !PlayerCharacter->HasAuthority() || !bHasExpectedEvent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	TargetCharacter = PlayerCharacter;
	CacheOriginalMovementState(PlayerCharacter);

	if (FartLaunchCueTag.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->AddGameplayCue(FartLaunchCueTag);
	}

	PlayerCharacter->AddActorWorldOffset(FVector(0.0f, 0.0f, 50.0f), false);

	MovementComponent->FallingLateralFriction = 0.0f;
	MovementComponent->SetMovementMode(EMovementMode::MOVE_Falling);

	ApplyCharacterImpulse();

	if (ReactionDuration <= 0.0f)
	{
		EndImpulseBehavior();
	}
	else
	{
		World->GetTimerManager().SetTimer(DurationTimerHandle, this, &UGA_CPPlayerFartLaunch::EndImpulseBehavior, ReactionDuration, false);
	}
}

void UGA_CPPlayerFartLaunch::EndImpulseBehavior()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_CPPlayerFartLaunch::ApplyCharacterImpulse()
{
	ACharacter* PlayerCharacter = TargetCharacter.Get();
	UCharacterMovementComponent* MovementComponent = IsValid(PlayerCharacter) ? PlayerCharacter->GetCharacterMovement() : nullptr;
	if (!IsValid(MovementComponent))
	{
		EndImpulseBehavior();
		return;
	}

	const FVector BaseImpulseDirection = PlayerCharacter->GetActorTransform().TransformVectorNoScale(LaunchRotation.Vector()).GetSafeNormal();
	const FVector ScatteredImpulseDirection = FMath::VRandCone(BaseImpulseDirection, FMath::DegreesToRadians(5.0f));
	MovementComponent->AddImpulse(ScatteredImpulseDirection * LaunchForce, true);
}

void UGA_CPPlayerFartLaunch::CacheOriginalMovementState(ACharacter* InTargetCharacter)
{
	bHasCachedOriginalMovementState = false;
	UCharacterMovementComponent* MovementComponent = IsValid(InTargetCharacter) ? InTargetCharacter->GetCharacterMovement() : nullptr;
	if (!IsValid(MovementComponent))
	{
		return;
	}

	OriginalMovementMode = MovementComponent->MovementMode;
	OriginalCustomMovementMode = MovementComponent->CustomMovementMode;
	OriginalFallingLateralFriction = MovementComponent->FallingLateralFriction;
	bHasCachedOriginalMovementState = true;
}

void UGA_CPPlayerFartLaunch::RestoreOriginalMovementState(ACharacter* InTargetCharacter)
{
	if (!IsValid(InTargetCharacter) || !bHasCachedOriginalMovementState)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = InTargetCharacter->GetCharacterMovement())
	{
		MovementComponent->FallingLateralFriction = OriginalFallingLateralFriction;

		const bool bWasGroundMovement = OriginalMovementMode == MOVE_Walking || OriginalMovementMode == MOVE_NavWalking;
		const EMovementMode RestoredMovementMode = bWasGroundMovement && !MovementComponent->IsMovingOnGround() ? MOVE_Falling : OriginalMovementMode;
		MovementComponent->SetMovementMode(RestoredMovementMode, OriginalCustomMovementMode);
	}

	bHasCachedOriginalMovementState = false;
}

void UGA_CPPlayerFartLaunch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DurationTimerHandle);
	}

	if (FartLaunchCueTag.IsValid() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveGameplayCue(FartLaunchCueTag);
	}

	RestoreOriginalMovementState(TargetCharacter.Get());
	TargetCharacter.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

const FGameplayTag& UGA_CPPlayerFartLaunch::GetFartLaunchEventTag()
{
	static const FGameplayTag FartLaunchEventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Potion.Effect.FartLaunch")));
	return FartLaunchEventTag;
}
