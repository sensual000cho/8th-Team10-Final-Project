#include "NPC/GA/CPGA_FartLaunch.h"
#include "NPC/CPBaseNPC.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Data/NPC/CPNPCDataAsset.h"

UCPGA_FartLaunch::UCPGA_FartLaunch()
{
	ImmunityTag = FGameplayTag::RequestGameplayTag(FName("Immunity.Potion.FartLaunch"));
}

void UCPGA_FartLaunch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACPBaseNPC* TargetCharacter = PrepareReactionNPC(Handle, ActorInfo, ActivationInfo);
	if (!TargetCharacter) { return; }

	if (FartLaunchCueTag.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->AddGameplayCue(FartLaunchCueTag);
	}

	TargetCharacter->AddActorWorldOffset(FVector(0.f, 0.f, 50.f), false);

	if (UCharacterMovementComponent* MovementComp = TargetCharacter->GetCharacterMovement())
	{
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

	if (const UCPNPCDataAsset* NPCData = TargetCharacter->GetNPCData())
	{
		if (UAnimSequence* LaunchAnim = NPCData->LaunchAnimation.LoadSynchronous())
		{
			if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
			{
				if (UAnimInstance* AnimInst = Mesh->GetAnimInstance())
				{
					AnimInst->PlaySlotAnimationAsDynamicMontage(LaunchAnim, FName("DefaultSlot"), 0.15f, 0.15f, 1.0f);
				}
				else
				{
					Mesh->PlayAnimation(LaunchAnim, true);
				}
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

	if (UCapsuleComponent* Capsule = TargetCharacter->GetCapsuleComponent())
	{
		Capsule->OnComponentHit.RemoveDynamic(this, &UCPGA_FartLaunch::OnCapsuleHit);
	}

	EnterRagdoll(TargetCharacter);

	if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
	{
		if (!Mesh->IsSimulatingPhysics())
		{
			Mesh->SetSimulatePhysics(true);
		}

		FVector CurrentVel = TargetCharacter->GetVelocity();
		if (CurrentVel.IsNearlyZero()) { CurrentVel = TargetCharacter->GetActorForwardVector() * LaunchForce; }
		Mesh->SetPhysicsLinearVelocity(CurrentVel);
		Mesh->SetPhysicsAngularVelocityInDegrees(FMath::VRand() * 360.f);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ThrustTimerHandle, this, &UCPGA_FartLaunch::ApplyRagdollThrust, 0.1f, true, 0.02f);
	}
}

void UCPGA_FartLaunch::ApplyRagdollThrust()
{
	USkeletalMeshComponent* Mesh = GetRagdollingMesh();
	if (!Mesh) { return; }

	if (!Mesh->IsSimulatingPhysics())
	{
		Mesh->SetSimulatePhysics(true);
	}

	FTransform PelvisTransform = Mesh->GetSocketTransform(FName("Fart_Socket"));
	const FVector BaseThrustDir = PelvisTransform.TransformVectorNoScale(LaunchRotation.Vector());
	const FVector ScatteredThrustDir = FMath::VRandCone(BaseThrustDir, FMath::DegreesToRadians(30.f));

	Mesh->AddImpulse(ScatteredThrustDir * LaunchForce, NAME_None, true);
	Mesh->AddAngularImpulseInDegrees(FMath::VRand() * ErraticSpinForce, NAME_None, true);
}

void UCPGA_FartLaunch::EndFloatBehavior()
{
	TransitionToNaturalSettling(ThrustTimerHandle);
}

void UCPGA_FartLaunch::OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ClearTimers({ &RagdollTimerHandle });

	if (!bIsRagdolling)
	{
		StartRagdoll();
	}
}

void UCPGA_FartLaunch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearTimers({ &DurationTimerHandle, &RagdollTimerHandle, &ThrustTimerHandle });

	if (FartLaunchCueTag.IsValid() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveGameplayCue(FartLaunchCueTag);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}