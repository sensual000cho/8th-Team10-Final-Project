#include "Character/GAS/Abilities/CPGA_PlayerEnemyGust.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#include "Character/CPHealthComponent.h"
#include "Character/Enemy/CPEnemyCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"

UCPGA_PlayerEnemyGust::UCPGA_PlayerEnemyGust()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	const FGameplayTag EffectTag = FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Gust"));
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Reaction.Potion")));
	AssetTags.AddTag(EffectTag);
	SetAssetTags(AssetTags);
	ActivationOwnedTags.AddTag(EffectTag);
	ActivationBlockedTags.AddTag(EffectTag);
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.All")));
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Potion.Effect.Gust"));
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UCPGA_PlayerEnemyGust::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	ACPEnemyCharacter* Enemy = Cast<ACPEnemyCharacter>(Character);
	UCPHealthComponent* Health = Character ? Character->FindComponentByClass<UCPHealthComponent>() : nullptr;
	if (!IsValid(Character) || DurationSeconds <= 0.0f || WalkSpeedMultiplier < 1.0f || (Enemy && Enemy->IsDead()) || (Health && Health->IsDead()))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	CachedCharacter = Character;
	CachedMovement = Character->GetCharacterMovement();
	AppliedMultiplier = WalkSpeedMultiplier;
	if (Enemy)
	{
		HandlePreCharacterMovement();
		DyingDelegateHandle = Enemy->OnEnemyDying.AddUObject(this, &UCPGA_PlayerEnemyGust::HandleDurationExpired);
	}
	else
	{
		// Player BP가 매 프레임 속도를 다시 계산하므로 이동 직전에만 곱한다.
		const FObjectPropertyBase* Property = FindFProperty<FObjectPropertyBase>(Character->GetClass(), TEXT("AC_PreCMCTick"));
		UActorComponent* PreTick = Property ? Cast<UActorComponent>(Property->GetObjectPropertyValue_InContainer(Character)) : nullptr;
		FMulticastDelegateProperty* TickDelegate = PreTick ? FindFProperty<FMulticastDelegateProperty>(PreTick->GetClass(), TEXT("Tick")) : nullptr;
		if (!TickDelegate)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
			return;
		}
		CachedPreCMCTick = PreTick;
		FScriptDelegate Delegate;
		Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UCPGA_PlayerEnemyGust, HandlePreCharacterMovement));
		TickDelegate->AddDelegate(Delegate, PreTick, TickDelegate->ContainerPtrToValuePtr<void>(PreTick));
		Character->OnCharacterMovementUpdated.AddUniqueDynamic(this, &UCPGA_PlayerEnemyGust::RestoreCalculatedWalkSpeed);
		CachedHealth = Health;
		if (Health) Health->OnDeath.AddUniqueDynamic(this, &UCPGA_PlayerEnemyGust::HandlePlayerDeath);
	}
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (StartSFX) UGameplayStatics::PlaySoundAtLocation(this, StartSFX, Avatar->GetActorLocation());
	}
	bEffectSFXStarted = true;

	GetWorld()->GetTimerManager().SetTimer(DurationTimerHandle, this, &UCPGA_PlayerEnemyGust::HandleDurationExpired, DurationSeconds, false);
}

void UCPGA_PlayerEnemyGust::HandlePreCharacterMovement()
{
	if (!bSpeedApplied && CachedMovement.IsValid())
	{
		CachedMovement->MaxWalkSpeed *= AppliedMultiplier;
		bSpeedApplied = true;
	}
}

void UCPGA_PlayerEnemyGust::RemoveSpeedMultiplier()
{
	if (bSpeedApplied && CachedMovement.IsValid()) CachedMovement->MaxWalkSpeed /= AppliedMultiplier;
	bSpeedApplied = false;
}

void UCPGA_PlayerEnemyGust::RestoreCalculatedWalkSpeed(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	RemoveSpeedMultiplier();
}

void UCPGA_PlayerEnemyGust::HandleDurationExpired()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UCPGA_PlayerEnemyGust::HandlePlayerDeath(AActor* DamageCauser)
{
	HandleDurationExpired();
}

void UCPGA_PlayerEnemyGust::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bEffectSFXStarted)
	{
		bEffectSFXStarted = false;
		if (AActor* Avatar = GetAvatarActorFromActorInfo())
		{
			if (EndSFX) UGameplayStatics::PlaySoundAtLocation(this, EndSFX, Avatar->GetActorLocation());
		}
	}

	if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(DurationTimerHandle);
	if (UActorComponent* PreTick = CachedPreCMCTick.Get())
	{
		if (FMulticastDelegateProperty* TickDelegate = FindFProperty<FMulticastDelegateProperty>(PreTick->GetClass(), TEXT("Tick")))
		{
			FScriptDelegate Delegate;
			Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UCPGA_PlayerEnemyGust, HandlePreCharacterMovement));
			TickDelegate->RemoveDelegate(Delegate, PreTick, TickDelegate->ContainerPtrToValuePtr<void>(PreTick));
		}
	}
	if (ACharacter* Character = CachedCharacter.Get())
	{
		Character->OnCharacterMovementUpdated.RemoveDynamic(this, &UCPGA_PlayerEnemyGust::RestoreCalculatedWalkSpeed);
		if (ACPEnemyCharacter* Enemy = Cast<ACPEnemyCharacter>(Character)) Enemy->OnEnemyDying.Remove(DyingDelegateHandle);
	}
	if (UCPHealthComponent* Health = CachedHealth.Get()) Health->OnDeath.RemoveDynamic(this, &UCPGA_PlayerEnemyGust::HandlePlayerDeath);
	RemoveSpeedMultiplier();
	CachedCharacter.Reset();
	CachedMovement.Reset();
	CachedPreCMCTick.Reset();
	CachedHealth.Reset();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, false, bWasCancelled);
}
