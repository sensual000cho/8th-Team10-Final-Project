#include "Character/Enemy/GA/CPGA_EnemyFartLaunch.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "BrainComponent.h"
#include "Character/Enemy/CPEnemyCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

UCPGA_EnemyFartLaunch::UCPGA_EnemyFartLaunch()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer InitialAssetTags;
	InitialAssetTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Reaction.Potion"))));
	SetAssetTags(InitialAssetTags);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Immunity.Potion.All"))));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Immunity.Potion.FartLaunch"))));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("State.Reaction.Potion"))));

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Potion.Effect.FartLaunch")));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

}

void UCPGA_EnemyFartLaunch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACPEnemyCharacter* Enemy = ActorInfo ? Cast<ACPEnemyCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UWorld* World = GetWorld();
	if (!IsValid(Enemy) || Enemy->IsDead() || !World || !HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedEnemy = Enemy;
	CachedDamageCauser = TriggerEventData ? TriggerEventData->ContextHandle.GetInstigator() : nullptr;
	bLandingDamageApplied = false;
	bLaunchApplied = true;

	Enemy->CancelAttack();
	if (AAIController* AIController = Cast<AAIController>(Enemy->GetController()))
	{
		AIController->StopMovement();
		if (UBrainComponent* BrainComponent = AIController->GetBrainComponent())
		{
			BrainComponent->PauseLogic(TEXT("Enemy FartLaunch"));
		}
	}

	UCharacterMovementComponent* CharacterMovement = Enemy->GetCharacterMovement();
	OriginalMovementMode = CharacterMovement->MovementMode;
	OriginalCustomMovementMode = CharacterMovement->CustomMovementMode;
	OriginalGravityScale = CharacterMovement->GravityScale;
	OriginalFallingLateralFriction = CharacterMovement->FallingLateralFriction;
	CharacterMovement->StopMovementImmediately();
	CharacterMovement->GravityScale = 0.0f;
	CharacterMovement->FallingLateralFriction = 0.0f;
	CharacterMovement->SetMovementMode(MOVE_Falling);
	Enemy->LandedDelegate.AddUniqueDynamic(this, &UCPGA_EnemyFartLaunch::HandleLanded);

	USkeletalMeshComponent* Mesh = Enemy->GetMesh();
	OriginalAnimInstanceClass = Mesh->GetAnimClass();
	OriginalMeshRelativeRotation = Mesh->GetRelativeRotation();
	if (IsValid(LaunchAnimation))
	{
		Mesh->PlayAnimation(LaunchAnimation, true);
	}

	if (FartLaunchCueTag.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->AddGameplayCue(FartLaunchCueTag);
	}

	Enemy->AddActorWorldOffset(FVector(0.0f, 0.0f, 50.0f), false);
	Enemy->LaunchCharacter(FVector::UpVector * LaunchForce, true, true);

	World->GetTimerManager().SetTimer(RiseTimerHandle, this, &UCPGA_EnemyFartLaunch::BeginFall, RiseDuration, false);
	World->GetTimerManager().SetTimer(VisualTumbleTimerHandle, this, &UCPGA_EnemyFartLaunch::ApplyVisualTumble, 0.08f, true);
}

void UCPGA_EnemyFartLaunch::BeginFall()
{
	if (ACPEnemyCharacter* Enemy = CachedEnemy.Get())
	{
		if (UCharacterMovementComponent* CharacterMovement = Enemy->GetCharacterMovement())
		{
			CharacterMovement->Velocity.Z = FMath::Min(CharacterMovement->Velocity.Z, 0.0f);
			CharacterMovement->GravityScale = OriginalGravityScale;
		}
	}

	if (FartLaunchCueTag.IsValid() && CurrentActorInfo && CurrentActorInfo->AbilitySystemComponent.IsValid())
	{
		CurrentActorInfo->AbilitySystemComponent->RemoveGameplayCue(FartLaunchCueTag);
	}
}

void UCPGA_EnemyFartLaunch::HandleLanded(const FHitResult& Hit)
{
	ApplyLandingDamage();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCPGA_EnemyFartLaunch::ApplyLandingDamage()
{
	ACPEnemyCharacter* Enemy = CachedEnemy.Get();
	if (bLandingDamageApplied || !IsValid(Enemy))
	{
		return;
	}

	bLandingDamageApplied = true;
	GetWorld()->GetTimerManager().ClearTimer(VisualTumbleTimerHandle);
	Enemy->ApplyDamage(1.0f, CachedDamageCauser.Get());
}

void UCPGA_EnemyFartLaunch::ApplyVisualTumble()
{
	ACPEnemyCharacter* Enemy = CachedEnemy.Get();
	USkeletalMeshComponent* Mesh = IsValid(Enemy) ? Enemy->GetMesh() : nullptr;
	if (!IsValid(Mesh) || bLandingDamageApplied)
	{
		return;
	}

	const FRotator TumbleDelta(
		FMath::FRandRange(-TumbleDegreesPerStep, TumbleDegreesPerStep),
		FMath::FRandRange(-TumbleDegreesPerStep, TumbleDegreesPerStep),
		FMath::FRandRange(-TumbleDegreesPerStep, TumbleDegreesPerStep));
	Mesh->AddLocalRotation(TumbleDelta);
}

void UCPGA_EnemyFartLaunch::RestoreLivingState()
{
	ACPEnemyCharacter* Enemy = CachedEnemy.Get();
	if (!IsValid(Enemy) || Enemy->IsDead())
	{
		return;
	}

	if (UCharacterMovementComponent* CharacterMovement = Enemy->GetCharacterMovement())
	{
		CharacterMovement->GravityScale = OriginalGravityScale;
		CharacterMovement->FallingLateralFriction = OriginalFallingLateralFriction;
		CharacterMovement->SetMovementMode(OriginalMovementMode, OriginalCustomMovementMode);
	}

	if (USkeletalMeshComponent* Mesh = Enemy->GetMesh())
	{
		Mesh->SetRelativeRotation(OriginalMeshRelativeRotation);
		Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		Mesh->SetAnimInstanceClass(OriginalAnimInstanceClass);
	}

	if (AAIController* AIController = Cast<AAIController>(Enemy->GetController()))
	{
		if (UBrainComponent* BrainComponent = AIController->GetBrainComponent())
		{
			BrainComponent->ResumeLogic(TEXT("Enemy FartLaunch ended"));
		}
	}
}

void UCPGA_EnemyFartLaunch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RiseTimerHandle);
		World->GetTimerManager().ClearTimer(VisualTumbleTimerHandle);
	}

	if (ACPEnemyCharacter* Enemy = CachedEnemy.Get())
	{
		Enemy->LandedDelegate.RemoveDynamic(this, &UCPGA_EnemyFartLaunch::HandleLanded);
	}

	if (FartLaunchCueTag.IsValid() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveGameplayCue(FartLaunchCueTag);
	}

	if (bLaunchApplied)
	{
		RestoreLivingState();
	}

	bLaunchApplied = false;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
