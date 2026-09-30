#include "Character/GAS/Abilities/CPGA_PlayerSlime.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#include "AbilitySystemComponent.h"
#include "Character/CPCarryComponent.h"
#include "Character/CPHealthComponent.h"
#include "Character/CPInteractionComponent.h"
#include "Character/CPPlayerController.h"
#include "Character/GAS/Abilities/GA_CPPlayerFartLaunch.h"
#include "Components/CPInventoryComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

UCPGA_PlayerSlime::UCPGA_PlayerSlime()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	const FGameplayTag EffectTag = FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Slime"));
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Reaction.Potion")));
	AssetTags.AddTag(EffectTag);
	SetAssetTags(AssetTags);
	ActivationOwnedTags.AddTag(EffectTag);
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.FartLaunch")));
	ActivationBlockedTags.AddTag(EffectTag);
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.All")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.Slime")));
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Potion.Effect.Slime"));
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

bool UCPGA_PlayerSlime::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;
	const ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!Character || !SlimeMesh || !SlimeAnimClass || DurationSeconds <= 0.0f) return false;
	const UCPHealthComponent* Health = Character->FindComponentByClass<UCPHealthComponent>();
	if (Health && Health->IsDead()) return false;
	// 이미 시작된 불방구는 취소하지 않고, 그동안 Slime 적용을 무시한다.
	for (const FGameplayAbilitySpec& Spec : ActorInfo->AbilitySystemComponent->GetActivatableAbilities())
	{
		if (Spec.IsActive() && Spec.Ability && Spec.Ability->IsA<UGA_CPPlayerFartLaunch>()) return false;
	}
	return true;
}

void UCPGA_PlayerSlime::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	UCPCarryComponent* Carry = Character->FindComponentByClass<UCPCarryComponent>();
	if (Carry && Carry->HasHeldProp())
	{
		ACPPlayerController* Controller = Cast<ACPPlayerController>(Character->GetController());
		if (Controller) Controller->OnCarryHeldProp(FInputActionValue());
		else Carry->ConsumeHeldPropToInventory(Character->FindComponentByClass<UCPInventoryComponent>());
		// 수납할 수 없으면 물건을 없애지 않고 기존 내려놓기 경로를 사용한다.
		if (Carry->HasHeldProp()) Carry->ResetCarryState();
	}

	CachedInteraction = Character->FindComponentByClass<UCPInteractionComponent>();
	if (CachedInteraction.IsValid())
	{
		bOriginalInteractionBlocked = CachedInteraction->IsInteractionBlocked();
		CachedInteraction->SetInteractionBlocked(true);
	}

	TArray<UMeshComponent*> Meshes;
	Character->GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* Mesh : Meshes)
	{
		if (!Mesh->bHiddenInGame)
		{
			HiddenMeshes.Add(Mesh);
			Mesh->SetHiddenInGame(true);
		}
	}

	SlimeVisual = NewObject<USkeletalMeshComponent>(Character);
	SlimeVisual->SetupAttachment(Character->GetMesh());
	SlimeVisual->SetRelativeTransform(SlimeRelativeTransform);
	SlimeVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SlimeVisual->SetSkeletalMesh(SlimeMesh);
	SlimeVisual->SetAnimInstanceClass(SlimeAnimClass);
	if (SlimeMaterial) SlimeVisual->SetMaterial(0, SlimeMaterial);
	SlimeVisual->RegisterComponent();
	// 위치와 크기는 Giant를 따르되 회전만 부모 Mesh에서 분리한다.
	const FQuat InitialRotation = SlimeVisual->GetComponentQuat();
	SlimeVisual->SetAbsolute(false, true, false);
	SlimeVisual->SetWorldRotation(InitialRotation);
	CachedCharacter = Character;
	Character->OnCharacterMovementUpdated.AddUniqueDynamic(this, &UCPGA_PlayerSlime::UpdateSlimeFacing);

	CachedHealth = Character->FindComponentByClass<UCPHealthComponent>();
	if (CachedHealth.IsValid()) CachedHealth->OnDeath.AddUniqueDynamic(this, &UCPGA_PlayerSlime::HandlePlayerDeath);
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (StartSFX) UGameplayStatics::PlaySoundAtLocation(this, StartSFX, Avatar->GetActorLocation());
	}
	bEffectSFXStarted = true;

	GetWorld()->GetTimerManager().SetTimer(DurationTimerHandle, this, &UCPGA_PlayerSlime::HandleDurationExpired, DurationSeconds, false);
}

void UCPGA_PlayerSlime::UpdateSlimeFacing(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	if (!IsValid(SlimeVisual) || !CachedCharacter.IsValid()) return;
	const FVector MoveDirection = CachedCharacter->GetVelocity().GetSafeNormal2D();
	if (MoveDirection.IsNearlyZero()) return;
	const FQuat TargetRotation = MoveDirection.ToOrientationQuat() * CachedCharacter->GetMesh()->GetRelativeRotation().Quaternion() * SlimeRelativeTransform.GetRotation();
	SlimeVisual->SetWorldRotation(FMath::QInterpTo(SlimeVisual->GetComponentQuat(), TargetRotation, DeltaSeconds, 10.0f));
}

void UCPGA_PlayerSlime::HandleDurationExpired()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UCPGA_PlayerSlime::HandlePlayerDeath(AActor* DamageCauser)
{
	HandleDurationExpired();
}

void UCPGA_PlayerSlime::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
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
	if (CachedCharacter.IsValid()) CachedCharacter->OnCharacterMovementUpdated.RemoveDynamic(this, &UCPGA_PlayerSlime::UpdateSlimeFacing);
	CachedCharacter.Reset();
	if (IsValid(SlimeVisual)) SlimeVisual->DestroyComponent();
	SlimeVisual = nullptr;
	for (const TWeakObjectPtr<UMeshComponent>& Mesh : HiddenMeshes)
	{
		if (Mesh.IsValid()) Mesh->SetHiddenInGame(false);
	}
	HiddenMeshes.Reset();
	if (CachedInteraction.IsValid()) CachedInteraction->SetInteractionBlocked(bOriginalInteractionBlocked);
	if (CachedHealth.IsValid()) CachedHealth->OnDeath.RemoveDynamic(this, &UCPGA_PlayerSlime::HandlePlayerDeath);
	CachedInteraction.Reset();
	CachedHealth.Reset();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, false, bWasCancelled);
}
