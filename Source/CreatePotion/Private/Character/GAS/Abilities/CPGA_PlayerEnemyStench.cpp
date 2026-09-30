#include "Character/GAS/Abilities/CPGA_PlayerEnemyStench.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#include "Character/CPHealthComponent.h"
#include "Character/Enemy/CPEnemyCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"

UCPGA_PlayerEnemyStench::UCPGA_PlayerEnemyStench()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	const FGameplayTag EffectTag = FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Stench"));
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Reaction.Potion")));
	AssetTags.AddTag(EffectTag);
	SetAssetTags(AssetTags);
	ActivationOwnedTags.AddTag(EffectTag);
	ActivationBlockedTags.AddTag(EffectTag);
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.All")));
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Potion.Effect.Stench"));
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

bool UCPGA_PlayerEnemyStench::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;
	const ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!IsValid(Character) || !StenchNiagaraSystem || DurationSeconds <= 0.0f) return false;
	const ACPEnemyCharacter* Enemy = Cast<ACPEnemyCharacter>(Character);
	const UCPHealthComponent* Health = Character->FindComponentByClass<UCPHealthComponent>();
	return !(Enemy && Enemy->IsDead()) && !(Health && Health->IsDead());
}

void UCPGA_PlayerEnemyStench::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	CachedCharacter = Character;
	StenchVisual = UNiagaraFunctionLibrary::SpawnSystemAttached(StenchNiagaraSystem, Character->GetRootComponent(), NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false, false, ENCPoolMethod::None, false);
	if (StenchVisual)
	{
		StenchVisual->SetAbsolute(false, true, true);
		UpdateVisual();
		StenchVisual->Activate(true);
	}
	Character->OnEndPlay.AddUniqueDynamic(this, &UCPGA_PlayerEnemyStench::HandleOwnerEndPlay);
	if (ACPEnemyCharacter* Enemy = Cast<ACPEnemyCharacter>(Character))
	{
		DyingDelegateHandle = Enemy->OnEnemyDying.AddUObject(this, &UCPGA_PlayerEnemyStench::HandleDurationExpired);
	}
	else
	{
		CachedHealth = Character->FindComponentByClass<UCPHealthComponent>();
		if (CachedHealth.IsValid()) CachedHealth->OnDeath.AddUniqueDynamic(this, &UCPGA_PlayerEnemyStench::HandlePlayerDeath);
	}
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (StartSFX) UGameplayStatics::PlaySoundAtLocation(this, StartSFX, Avatar->GetActorLocation());
	}
	bEffectSFXStarted = true;

	GetWorld()->GetTimerManager().SetTimer(DurationTimerHandle, this, &UCPGA_PlayerEnemyStench::HandleDurationExpired, DurationSeconds, false);
	GetWorld()->GetTimerManager().SetTimer(VisualTimerHandle, this, &UCPGA_PlayerEnemyStench::UpdateVisual, 0.1f, true);
}

void UCPGA_PlayerEnemyStench::UpdateVisual()
{
	ACharacter* Character = CachedCharacter.Get();
	if (!Character || !IsValid(StenchVisual)) return;
	USkeletalMeshComponent* Body = Character->GetMesh();
	if (!Body) return;
	if (Body->bHiddenInGame)
	{
		TArray<USkeletalMeshComponent*> Meshes;
		Character->GetComponents<USkeletalMeshComponent>(Meshes);
		for (USkeletalMeshComponent* Mesh : Meshes)
		{
			if (Mesh->GetAttachParent() == Body && !Mesh->bHiddenInGame && Mesh->GetSkeletalMeshAsset())
			{
				Body = Mesh;
				break;
			}
		}
	}
	// 숨긴 사람 메시와 독립적으로 현재 외형 중앙에 배치한다.
	StenchVisual->SetWorldLocation(Body->Bounds.Origin);
	StenchVisual->SetVariableFloat(TEXT("Scale"), VisualScale * Body->GetComponentScale().GetAbs().GetMax());
}

void UCPGA_PlayerEnemyStench::HandleDurationExpired()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UCPGA_PlayerEnemyStench::HandlePlayerDeath(AActor* DamageCauser)
{
	HandleDurationExpired();
}

void UCPGA_PlayerEnemyStench::HandleOwnerEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	HandleDurationExpired();
}

void UCPGA_PlayerEnemyStench::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
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
		World->GetTimerManager().ClearTimer(VisualTimerHandle);
	}
	if (ACharacter* Character = CachedCharacter.Get())
	{
		Character->OnEndPlay.RemoveDynamic(this, &UCPGA_PlayerEnemyStench::HandleOwnerEndPlay);
		if (ACPEnemyCharacter* Enemy = Cast<ACPEnemyCharacter>(Character)) Enemy->OnEnemyDying.Remove(DyingDelegateHandle);
	}
	if (CachedHealth.IsValid()) CachedHealth->OnDeath.RemoveDynamic(this, &UCPGA_PlayerEnemyStench::HandlePlayerDeath);
	// 잔여 입자까지 즉시 제거하고 다음 적용 때 새로 만든다.
	if (IsValid(StenchVisual)) StenchVisual->DestroyComponent();
	StenchVisual = nullptr;
	CachedCharacter.Reset();
	CachedHealth.Reset();
	DyingDelegateHandle.Reset();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, false, bWasCancelled);
}
