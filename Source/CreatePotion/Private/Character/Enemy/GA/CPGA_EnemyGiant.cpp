#include "Character/Enemy/GA/CPGA_EnemyGiant.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#include "Character/Enemy/CPEnemyCharacter.h"
#include "TimerManager.h"

UCPGA_EnemyGiant::UCPGA_EnemyGiant()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Reaction.Potion")));
	// 기존 효과 태그로 중화 대상 목록에서 이 GA를 찾는다.
	AssetTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Giant")));
	SetAssetTags(AssetTags);
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.All")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.Giant")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Giant")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Giant")));

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Potion.Effect.Giant"));
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

bool UCPGA_EnemyGiant::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const ACPEnemyCharacter* Enemy = ActorInfo ? Cast<ACPEnemyCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	return IsValid(Enemy) && !Enemy->IsDead() && !bGiantApplied && Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UCPGA_EnemyGiant::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	ACPEnemyCharacter* Enemy = ActorInfo ? Cast<ACPEnemyCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!IsValid(Enemy) || Enemy->IsDead() || DurationSeconds <= 0.0f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	CachedEnemy = Enemy;
	bGiantApplied = true;
	Enemy->SetGiantMultipliers(ScaleMultiplier, WalkSpeedMultiplier, AttackImpulseMultiplier);
	DyingDelegateHandle = Enemy->OnEnemyDying.AddUObject(this, &UCPGA_EnemyGiant::HandleDurationExpired);
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (StartSFX) UGameplayStatics::PlaySoundAtLocation(this, StartSFX, Avatar->GetActorLocation());
	}
	bEffectSFXStarted = true;

	GetWorld()->GetTimerManager().SetTimer(DurationTimerHandle, this, &UCPGA_EnemyGiant::HandleDurationExpired, DurationSeconds, false);
}

void UCPGA_EnemyGiant::HandleDurationExpired()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UCPGA_EnemyGiant::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
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
	if (ACPEnemyCharacter* Enemy = CachedEnemy.Get())
	{
		Enemy->OnEnemyDying.Remove(DyingDelegateHandle);
		if (bGiantApplied) Enemy->SetGiantMultipliers(1.0f, 1.0f, 1.0f);
	}
	bGiantApplied = false;
	CachedEnemy.Reset();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, false, bWasCancelled);
}
