#include "NPC/GA/CPGA_FartLaunch.h"
#include "NPC/CPBaseNPC.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "GameInstance/Subsystem/CPNPCSubsystem.h"

UCPGA_FartLaunch::UCPGA_FartLaunch()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ImmunityTag = FGameplayTag::RequestGameplayTag(FName("Immunity.Potion.FartLaunch"));
}

void UCPGA_FartLaunch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACPBaseNPC* TargetCharacter = GetValidatedOwningPotionNPC(Handle, ActorInfo, ActivationInfo);
	if (!TargetCharacter) return;

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

	// 임시 프로토용 날아가는 애니메이션 추가
	if (TargetCharacter->NPCData)
	{
		if (UAnimSequence* LaunchAnim = TargetCharacter->NPCData->LaunchAnimation.LoadSynchronous())
		{
			if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
			{
				Mesh->PlayAnimation(LaunchAnim, true);
			}
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RagdollTimerHandle, this, &UCPGA_FartLaunch::StartRagdoll, RagdollDelay, false);
		World->GetTimerManager().SetTimer(DurationTimerHandle, this, &UCPGA_FartLaunch::EndFloatBehavior, FloatDuration, false);
	}
}

void UCPGA_FartLaunch::StartRagdoll()
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

		FVector CurrentVel = TargetCharacter->GetVelocity();
		if (CurrentVel.IsNearlyZero()) { CurrentVel = TargetCharacter->GetActorForwardVector() * LaunchForce; }
		Mesh->SetPhysicsLinearVelocity(CurrentVel);
		Mesh->SetPhysicsAngularVelocityInDegrees(FMath::VRand() * 360.f);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ThrustTimerHandle, this, &UCPGA_FartLaunch::ApplyRagdollThrust, 0.1f, true);
		}
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
			World->GetTimerManager().SetTimer(FailsafeTimerHandle, this, &UCPGA_FartLaunch::ForceEndAbility, MaxFailsafeTime, false);
		}
	}
	else
	{
		ForceEndAbility();
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

void UCPGA_FartLaunch::OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RagdollTimerHandle);
	}

	if (!bIsRagdolling)
	{
		StartRagdoll();
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
			ForceEndAbility();
		}
	}
}

void UCPGA_FartLaunch::ForceEndAbility()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCPGA_FartLaunch::ApplyRagdollThrust()
{
	ACPBaseNPC* TargetCharacter = GetOwningPotionNPC(CurrentActorInfo);
	if (!IsValid(TargetCharacter) || !bIsRagdolling) { return; }

	if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
	{
		FTransform PelvisTransform = Mesh->GetSocketTransform(FName("Fart_Socket"));
		const FVector BaseThrustDir = PelvisTransform.TransformVectorNoScale(LaunchRotation.Vector());
		const FVector ScatteredThrustDir = FMath::VRandCone(BaseThrustDir, FMath::DegreesToRadians(30.f));

		Mesh->AddImpulse(ScatteredThrustDir * LaunchForce, NAME_None, true);
		Mesh->AddAngularImpulseInDegrees(FMath::VRand() * ErraticSpinForce, NAME_None, true);
	}
}

void UCPGA_FartLaunch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DurationTimerHandle);
		World->GetTimerManager().ClearTimer(RagdollTimerHandle);
		World->GetTimerManager().ClearTimer(VelocityCheckTimerHandle);
		World->GetTimerManager().ClearTimer(ThrustTimerHandle);
		World->GetTimerManager().ClearTimer(FailsafeTimerHandle);
	}

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (ACPBaseNPC* TargetCharacter = Cast<ACPBaseNPC>(ActorInfo->AvatarActor.Get()))
		{
			UCharacterMovementComponent* MovementComp = TargetCharacter->GetCharacterMovement();
			UCapsuleComponent* Capsule = TargetCharacter->GetCapsuleComponent();
			USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh();

			if (MovementComp)
			{
				MovementComp->GravityScale = OriginalGravityScale;
			}
			if (Capsule)
			{
				Capsule->OnComponentHit.RemoveDynamic(this, &UCPGA_FartLaunch::OnCapsuleHit);
			}
			if (Mesh)
			{
				Mesh->SetEnableGravity(true);
				Mesh->OnComponentHit.RemoveDynamic(this, &UCPGA_FartLaunch::OnMeshHit);
			}

			if (bIsRagdolling && Mesh && Capsule)
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
				}
				TargetCharacter->ReapplyActivePotionVisuals();
			}


			if (TargetCharacter->NPCData && Mesh)
			{
				if (UAnimSequence* IdleAnim = TargetCharacter->NPCData->IdleAnimation.LoadSynchronous())
				{
					Mesh->PlayAnimation(IdleAnim, true);
				}
			}
		}
	}

	if (FartLaunchCueTag.IsValid() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveGameplayCue(FartLaunchCueTag);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}