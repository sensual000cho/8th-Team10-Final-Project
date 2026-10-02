#include "NPC/GA/CPGA_ImpulseReactionBase.h"
#include "NPC/CPBaseNPC.h"
#include "NPC/CPQuestNPC.h"
#include "NPC/Components/CPNPCVisualComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"

UCPGA_ImpulseReactionBase::UCPGA_ImpulseReactionBase()
{
}

ACPBaseNPC* UCPGA_ImpulseReactionBase::PrepareReactionNPC(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	ACPBaseNPC* TargetCharacter = GetValidatedOwningPotionNPC(Handle, ActorInfo, ActivationInfo);
	if (!TargetCharacter) return nullptr;

	if (ACPQuestNPC* QuestNPC = Cast<ACPQuestNPC>(TargetCharacter))
	{
		QuestNPC->SetQuestMarkerVisibility(false);
	}

	if (ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.NPC.Displaced")));
	}

	if (UCapsuleComponent* Capsule = TargetCharacter->GetCapsuleComponent())
	{
		OriginalCapsuleCollisionProfile = Capsule->GetCollisionProfileName();
		OriginalCapsuleCollisionEnabled = Capsule->GetCollisionEnabled();
	}

	if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
	{
		OriginalMeshCollisionProfile = Mesh->GetCollisionProfileName();
		OriginalMeshCollisionEnabled = Mesh->GetCollisionEnabled();
	}

	if (UCharacterMovementComponent* MovementComp = TargetCharacter->GetCharacterMovement())
	{
		OriginalGravityScale = MovementComp->GravityScale;
	}

	bIsRagdolling = false;
	bReactToMeshBounce = false;
	LastMeshHitTime = 0.0f;

	return TargetCharacter;
}

void UCPGA_ImpulseReactionBase::EnterRagdoll(ACPBaseNPC* TargetCharacter)
{
	if (!IsValid(TargetCharacter)) return;

	if (UCapsuleComponent* Capsule = TargetCharacter->GetCapsuleComponent())
	{
		Capsule->SetNotifyRigidBodyCollision(false);
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
	{
		bIsRagdolling = true;
		bReactToMeshBounce = true;
		Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
		Mesh->SetSimulatePhysics(true);
		Mesh->SetEnableGravity(false);
		Mesh->WakeAllRigidBodies();
		Mesh->SetNotifyRigidBodyCollision(true);
		Mesh->SetUseCCD(true);
		Mesh->OnComponentHit.AddUniqueDynamic(this, &UCPGA_ImpulseReactionBase::OnRagdollMeshHit);
	}
}

USkeletalMeshComponent* UCPGA_ImpulseReactionBase::GetRagdollingMesh() const
{
	ACPBaseNPC* TargetCharacter = GetOwningPotionNPC(CurrentActorInfo);
	if (!IsValid(TargetCharacter) || !bIsRagdolling)
	{
		return nullptr;
	}

	return TargetCharacter->GetMesh();
}

void UCPGA_ImpulseReactionBase::OnRagdollMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bReactToMeshBounce) return;

	UWorld* World = GetWorld();
	if (!World) return; 

	const float CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastMeshHitTime < MeshHitDebounceInterval) return;
	LastMeshHitTime = CurrentTime;

	if (USkeletalMeshComponent* Mesh = GetRagdollingMesh())
	{
		ProcessMeshBounce(Mesh, Hit, GetMeshBounceMinSpeed(), GetMeshBounceSpinMultiplier());
	}
}

void UCPGA_ImpulseReactionBase::StartVelocityCheckAndFailsafe(float StopThreshold, float FailsafeTime)
{
	StopVelocityThreshold = StopThreshold;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(VelocityCheckTimerHandle, this, &UCPGA_ImpulseReactionBase::CheckRagdollVelocity, 0.2f, true);
		World->GetTimerManager().SetTimer(FailsafeTimerHandle, this, &UCPGA_ImpulseReactionBase::ForceEndAbility, FailsafeTime, false);
	}
}

void UCPGA_ImpulseReactionBase::CheckRagdollVelocity()
{
	if (USkeletalMeshComponent* Mesh = GetRagdollingMesh())
	{
		if (Mesh->GetComponentVelocity().SizeSquared() <= FMath::Square(StopVelocityThreshold))
		{
			ForceEndAbility();
		}
	}
}

void UCPGA_ImpulseReactionBase::ForceEndAbility()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCPGA_ImpulseReactionBase::ProcessMeshBounce(USkeletalMeshComponent* Mesh, const FHitResult& Hit, float MinBounceSpeed, float SpinForceMultiplier)
{
	if (!Mesh) return;

	FVector IncomingDir = Mesh->GetComponentVelocity().GetSafeNormal();
	if (IncomingDir.IsNearlyZero()) return;

	const FVector BounceDir = (FMath::GetReflectionVector(IncomingDir, Hit.ImpactNormal) + Hit.ImpactNormal * 0.5f).GetSafeNormal();
	const float NewSpeed = FMath::Max(Mesh->GetComponentVelocity().Size(), MinBounceSpeed);
	Mesh->SetPhysicsLinearVelocity(BounceDir * NewSpeed);

	if (SpinForceMultiplier > 0.0f)
	{
		Mesh->AddAngularImpulseInDegrees(FMath::VRand() * SpinForceMultiplier, NAME_None, true);
	}
}

void UCPGA_ImpulseReactionBase::TransitionToNaturalSettling(FTimerHandle& ActiveForceTimerHandle)
{
	ClearTimers({ &ActiveForceTimerHandle });

	if (USkeletalMeshComponent* Mesh = GetRagdollingMesh())
	{
		bReactToMeshBounce = false;
		Mesh->OnComponentHit.RemoveDynamic(this, &UCPGA_ImpulseReactionBase::OnRagdollMeshHit);

		Mesh->SetEnableGravity(true);

		StartVelocityCheckAndFailsafe(StopVelocityThreshold, MaxFailsafeTime);
	}
	else
	{
		ForceEndAbility();
	}
}

void UCPGA_ImpulseReactionBase::ClearTimers(const TArray<FTimerHandle*>& Handles)
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TM = World->GetTimerManager();
		for (FTimerHandle* Handle : Handles)
		{
			TM.ClearTimer(*Handle);
		}
	}
}

void UCPGA_ImpulseReactionBase::CleanupNPCReaction(const FGameplayAbilityActorInfo* ActorInfo)
{
	ClearTimers({ &VelocityCheckTimerHandle, &FailsafeTimerHandle });
	bReactToMeshBounce = false;

	if (ACPBaseNPC* TargetCharacter = GetOwningPotionNPC(ActorInfo))
	{
		if (UCharacterMovementComponent* MovementComp = TargetCharacter->GetCharacterMovement())
		{
			MovementComp->GravityScale = OriginalGravityScale;
			MovementComp->SetMovementMode(MOVE_Walking);
		}

		if (UCapsuleComponent* Capsule = TargetCharacter->GetCapsuleComponent())
		{
			Capsule->SetNotifyRigidBodyCollision(false);
			Capsule->SetCollisionProfileName(OriginalCapsuleCollisionProfile);
			Capsule->SetCollisionEnabled(OriginalCapsuleCollisionEnabled);
		}

		if (bIsRagdolling)
		{
			if (UCPNPCVisualComponent* VisualComp = TargetCharacter->GetVisualComponent())
			{
				VisualComp->RestoreMeshTransformAfterRagdoll();
			}
		}
		else
		{
			if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
			{
				Mesh->SetCollisionProfileName(OriginalMeshCollisionProfile);
				Mesh->SetCollisionEnabled(OriginalMeshCollisionEnabled);
				Mesh->OnComponentHit.RemoveDynamic(this, &UCPGA_ImpulseReactionBase::OnRagdollMeshHit);
			}
		}

		if (ACPQuestNPC* QuestNPC = Cast<ACPQuestNPC>(TargetCharacter))
		{
			QuestNPC->SetQuestMarkerVisibility(true);
		}
	}

	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.NPC.Displaced")));
	}
}

void UCPGA_ImpulseReactionBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	CleanupNPCReaction(ActorInfo);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}