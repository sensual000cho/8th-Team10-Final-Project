#include "Character/Enemy/GA/CPGA_EnemyFartLaunch.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "BrainComponent.h"
#include "Character/Enemy/CPEnemyCharacter.h"
#include "Components/SkeletalMeshComponent.h"

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
	CharacterMovement->StopMovementImmediately();
	CharacterMovement->SetMovementMode(MOVE_Falling);
	Enemy->LandedDelegate.AddUniqueDynamic(this, &UCPGA_EnemyFartLaunch::HandleLanded);

	USkeletalMeshComponent* Mesh = Enemy->GetMesh();
	OriginalAnimInstanceClass = Mesh->GetAnimClass();
	OriginalMeshRelativeRotation = Mesh->GetRelativeRotation();
	SpinAxis = FMath::VRand();
	SpinAngle = 0.0f;
	Enemy->OnCharacterMovementUpdated.AddUniqueDynamic(this, &UCPGA_EnemyFartLaunch::UpdateFlightSpin);
	if (IsValid(LaunchAnimation))
	{
		Mesh->PlayAnimation(LaunchAnimation, true);
	}

	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (StartSFX) UGameplayStatics::PlaySoundAtLocation(this, StartSFX, Avatar->GetActorLocation());
	}
	bEffectSFXStarted = true;
	if (FlightSFX) FlightAudioComponent = UGameplayStatics::SpawnSoundAttached(FlightSFX, Enemy->GetRootComponent(), NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true);

	if (FartLaunchCueTag.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->AddGameplayCue(FartLaunchCueTag);
	}

	const FVector LaunchDirection = FMath::VRandCone(FVector::UpVector, FMath::DegreesToRadians(LaunchConeHalfAngle));
	Enemy->LaunchCharacter(LaunchDirection * LaunchForce, true, true);
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
	Enemy->ApplyDamage(1.0f, CachedDamageCauser.Get());
}

void UCPGA_EnemyFartLaunch::UpdateFlightSpin(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	ACPEnemyCharacter* Enemy = CachedEnemy.Get();
	if (!IsValid(Enemy) || Enemy->IsDead()) return;

	SpinAngle = FMath::Fmod(SpinAngle + SpinSpeed * DeltaSeconds, 360.0f);
	const FQuat SpinRotation(SpinAxis, FMath::DegreesToRadians(SpinAngle));
	Enemy->GetMesh()->SetRelativeRotation(OriginalMeshRelativeRotation.Quaternion() * SpinRotation);
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

	if (ACPEnemyCharacter* Enemy = CachedEnemy.Get())
	{
		Enemy->LandedDelegate.RemoveDynamic(this, &UCPGA_EnemyFartLaunch::HandleLanded);
		Enemy->OnCharacterMovementUpdated.RemoveDynamic(this, &UCPGA_EnemyFartLaunch::UpdateFlightSpin);
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
