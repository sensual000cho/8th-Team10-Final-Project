#include "NPC/GA/CPGA_FartLaunch.h"
#include "NPC/CPBaseNPC.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Data/CPNPCDataAsset.h"
#include "GameInstance/Subsystem/CPNPCSubsystem.h"

UCPGA_FartLaunch::UCPGA_FartLaunch()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Immunity.Potion.FartLaunch")));
}

void UCPGA_FartLaunch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	//PrintPotionEventLog(TriggerEventData);

	ACPBaseNPC* TargetCharacter = GetOwningPotionNPC(ActorInfo);
	if (!IsValid(TargetCharacter) || !HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bIsRagdolling = false;
	LastMeshHitTime = 0.f;

	if (FartLaunchCueTag.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->AddGameplayCue(FartLaunchCueTag);
	}

	TargetCharacter->AddActorWorldOffset(FVector(0.f, 0.f, 50.f), false);

	if (UCharacterMovementComponent* MovementComp = TargetCharacter->GetCharacterMovement())
	{
		OriginalGravityScale = MovementComp->GravityScale;
		MovementComp->GravityScale = 0.0f;
		MovementComp->FallingLateralFriction = 0.0f;
		MovementComp->SetMovementMode(EMovementMode::MOVE_Falling);
	}

	if (UCapsuleComponent* Capsule = TargetCharacter->GetCapsuleComponent())
	{
		Capsule->SetNotifyRigidBodyCollision(true);
		Capsule->OnComponentHit.AddUniqueDynamic(this, &UCPGA_FartLaunch::OnCapsuleHit);
	}

	const FVector WorldStartDir = TargetCharacter->GetActorTransform().TransformVectorNoScale(LaunchRotation.Vector());
	TargetCharacter->LaunchCharacter(WorldStartDir * LaunchForce, true, true);

	GetWorld()->GetTimerManager().SetTimer(DurationTimerHandle, this, &UCPGA_FartLaunch::EndFloatBehavior, FloatDuration, false);
}

void UCPGA_FartLaunch::OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACPBaseNPC* TargetCharacter = GetOwningPotionNPC(CurrentActorInfo);
	if (!IsValid(TargetCharacter)) { return; }

	UCapsuleComponent* Capsule = TargetCharacter->GetCapsuleComponent();
	USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh();

	if (Capsule)
	{
		Capsule->OnComponentHit.RemoveDynamic(this, &UCPGA_FartLaunch::OnCapsuleHit);
		Capsule->SetNotifyRigidBodyCollision(false);
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (Mesh)
	{
		bIsRagdolling = true;
		Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
		Mesh->SetSimulatePhysics(true);
		Mesh->SetEnableGravity(false);
		Mesh->WakeAllRigidBodies();
		Mesh->SetNotifyRigidBodyCollision(true);
		Mesh->SetUseCCD(true);
		Mesh->OnComponentHit.AddUniqueDynamic(this, &UCPGA_FartLaunch::OnMeshHit);

		FVector IncomingDir = TargetCharacter->GetVelocity().GetSafeNormal();
		if (IncomingDir.IsNearlyZero()) { IncomingDir = TargetCharacter->GetActorForwardVector(); }

		const FVector BounceDir = (FMath::GetReflectionVector(IncomingDir, Hit.ImpactNormal) + Hit.ImpactNormal * 1.5f).GetSafeNormal();
		Mesh->SetPhysicsLinearVelocity(BounceDir * LaunchForce);
		Mesh->SetPhysicsAngularVelocityInDegrees(FMath::VRand() * 360.f);

		GetWorld()->GetTimerManager().SetTimer(ThrustTimerHandle, this, &UCPGA_FartLaunch::ApplyRagdollThrust, 0.1f, true);
	}
}

void UCPGA_FartLaunch::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastMeshHitTime < 0.1f) { return; }
	LastMeshHitTime = CurrentTime;

	ACPBaseNPC* TargetCharacter = GetOwningPotionNPC(CurrentActorInfo);
	if (!IsValid(TargetCharacter) || !bIsRagdolling) { return; }

	if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
	{
		FVector IncomingDir = Mesh->GetComponentVelocity().GetSafeNormal();
		if (IncomingDir.IsNearlyZero()) IncomingDir = TargetCharacter->GetActorForwardVector();

		const FVector BounceDir = (FMath::GetReflectionVector(IncomingDir, Hit.ImpactNormal) + Hit.ImpactNormal * 1.5f).GetSafeNormal();
		Mesh->SetPhysicsLinearVelocity(BounceDir * FMath::Max(Mesh->GetComponentVelocity().Size(), LaunchForce * 1.5f));
		Mesh->AddAngularImpulseInDegrees(FMath::VRand() * ErraticSpinForce, NAME_None, true);
	}
}

void UCPGA_FartLaunch::EndFloatBehavior()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(ThrustTimerHandle);
	}

	if (FartLaunchCueTag.IsValid() && CurrentActorInfo && CurrentActorInfo->AbilitySystemComponent.IsValid())
	{
		CurrentActorInfo->AbilitySystemComponent->RemoveGameplayCue(FartLaunchCueTag);
	}

	ACPBaseNPC* TargetCharacter = GetOwningPotionNPC(CurrentActorInfo);
	if (IsValid(TargetCharacter) && bIsRagdolling)
	{
		if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
		{
			Mesh->SetEnableGravity(true);
			Mesh->OnComponentHit.RemoveDynamic(this, &UCPGA_FartLaunch::OnMeshHit);
			Mesh->SetNotifyRigidBodyCollision(false);
		}

		if (World)
		{
			World->GetTimerManager().SetTimer(VelocityCheckTimerHandle, this, &UCPGA_FartLaunch::CheckRagdollVelocity, 0.2f, true);
		}
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UCPGA_FartLaunch::CheckRagdollVelocity()
{
	ACPBaseNPC* TargetCharacter = GetOwningPotionNPC(CurrentActorInfo);
	if (!IsValid(TargetCharacter) || !bIsRagdolling) { return; }

	if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
	{
		if (Mesh->GetComponentVelocity().SizeSquared() <= FMath::Square(StopVelocityThreshold))
		{
			GetWorld()->GetTimerManager().ClearTimer(VelocityCheckTimerHandle);
			RecoverFromRagdoll();
		}
	}
}

void UCPGA_FartLaunch::RecoverFromRagdoll()
{
	if (ACPBaseNPC* TargetCharacter = GetOwningPotionNPC(CurrentActorInfo))
	{
		USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh();
		UCapsuleComponent* Capsule = TargetCharacter->GetCapsuleComponent();

		if (Mesh && Capsule)
		{
			Capsule->SetWorldLocation(Mesh->GetSocketLocation(FName("pelvis")) + FVector(0.f, 0.f, Capsule->GetScaledCapsuleHalfHeight()));
			Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			Mesh->SetSimulatePhysics(false);
			Mesh->SetUseCCD(false);
			Mesh->SetCollisionProfileName(TEXT("CharacterMesh"));
			Mesh->AttachToComponent(Capsule, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			Mesh->SetRelativeLocation(FVector(0.f, 0.f, -Capsule->GetScaledCapsuleHalfHeight()));

			if (TargetCharacter->NPCData)
			{
				Mesh->SetRelativeRotation(TargetCharacter->NPCData->MeshRotationOffset);
				FVector FinalScale = TargetCharacter->NPCData->MeshScale;

				if (UGameInstance* GI = GetWorld()->GetGameInstance())
				{
					if (UCPNPCSubsystem* NPCSubsystem = GI->GetSubsystem<UCPNPCSubsystem>())
					{
						FCPNPCEffectSaveData SaveData;
						if (NPCSubsystem->GetNPCEffectData(TargetCharacter->GetPotionNPCId(), SaveData))
						{
							FGameplayTag GiantTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Giant"));
							if (const FCPActiveEffectInfo* EffectInfo = SaveData.ActiveEffects.Find(GiantTag))
							{
								FinalScale *= EffectInfo->Magnitude;
							}
						}
					}
				}
				Mesh->SetRelativeScale3D(FinalScale);
			}
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCPGA_FartLaunch::ApplyRagdollThrust()
{
	ACPBaseNPC* TargetCharacter = GetOwningPotionNPC(CurrentActorInfo);
	if (!IsValid(TargetCharacter) || !bIsRagdolling) { return; }

	if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
	{
		FTransform PelvisTransform = Mesh->GetSocketTransform(FName("pelvis"));
		const FVector BaseThrustDir = PelvisTransform.TransformVectorNoScale(LaunchRotation.Vector());
		const FVector ScatteredThrustDir = FMath::VRandCone(BaseThrustDir, FMath::DegreesToRadians(30.f));

		Mesh->AddImpulse(ScatteredThrustDir * LaunchForce, NAME_None, true);
		Mesh->AddAngularImpulseInDegrees(FMath::VRand() * ErraticSpinForce, NAME_None, true);
	}
}

void UCPGA_FartLaunch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (ACPBaseNPC* TargetCharacter = Cast<ACPBaseNPC>(ActorInfo->AvatarActor.Get()))
		{
			if (UCharacterMovementComponent* MovementComp = TargetCharacter->GetCharacterMovement())
			{
				MovementComp->GravityScale = OriginalGravityScale;
			}

			if (UCapsuleComponent* Capsule = TargetCharacter->GetCapsuleComponent())
			{
				Capsule->OnComponentHit.RemoveDynamic(this, &UCPGA_FartLaunch::OnCapsuleHit);
			}

			if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
			{
				Mesh->OnComponentHit.RemoveDynamic(this, &UCPGA_FartLaunch::OnMeshHit);
				Mesh->SetEnableGravity(true);
			}
		}
	}

	if (FartLaunchCueTag.IsValid() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveGameplayCue(FartLaunchCueTag);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DurationTimerHandle);
		World->GetTimerManager().ClearTimer(VelocityCheckTimerHandle);
		World->GetTimerManager().ClearTimer(ThrustTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}