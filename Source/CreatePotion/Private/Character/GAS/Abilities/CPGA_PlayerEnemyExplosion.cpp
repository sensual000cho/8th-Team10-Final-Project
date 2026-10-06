#include "Character/GAS/Abilities/CPGA_PlayerEnemyExplosion.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Character/CPHealthComponent.h"
#include "Character/CPInteractionComponent.h"
#include "Character/Enemy/CPEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

namespace
{
	struct FSharedCueSlot
	{
		TWeakObjectPtr<const UWorld> World;
		FVector Location = FVector::ZeroVector;
		double Time = -1000.0;
	};


	FSharedCueSlot GatherSlot;
	FSharedCueSlot ExplodeSlot;

	bool TryClaimSharedCue(const UWorld* World, const FVector& Loc, FSharedCueSlot& Slot)
	{
		const double Now = World->GetTimeSeconds();
		if (Slot.World.Get() == World
			&& Now - Slot.Time < 0.3
			&& FVector::DistSquared(Loc, Slot.Location) < 1.0)
		{
			return false;
		}

		Slot.World = World;
		Slot.Time = Now;
		Slot.Location = Loc;
		return true;
	}

	void ExecuteSharedCue(UAbilitySystemComponent* ASC, const UWorld* World, const FGameplayTag& Tag, const FVector& Center, float Radius, FSharedCueSlot& Slot)
	{
		if (!ASC || !World || !Tag.IsValid() || !TryClaimSharedCue(World, Center, Slot)) return;

		FGameplayCueParameters Params;
		Params.Location = Center;
		Params.RawMagnitude = Radius;
		ASC->ExecuteGameplayCue(Tag, Params);
	}

}

UCPGA_PlayerEnemyExplosion::UCPGA_PlayerEnemyExplosion()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	FGameplayTagContainer Tags;
	Tags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Reaction.Potion")));
	SetAssetTags(Tags);
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Reaction.Potion")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.Explosion")));
	// 래그돌 중 형상·이동을 바꾸는 반응만 차단하고 Reset은 허용
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.FartLaunch")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.Giant")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.Slime")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.All")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.Explosion")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Reaction.Potion")));
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Potion.Effect.Explosion"));
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

USkeletalMeshComponent* UCPGA_PlayerEnemyExplosion::FindVisualMesh(ACharacter* Character) const
{
	USkeletalMeshComponent* Body = Character ? Character->GetMesh() : nullptr;
	if (Body && Body->bHiddenInGame)
	{
		TArray<USkeletalMeshComponent*> Meshes;
		Character->GetComponents(Meshes);
		for (USkeletalMeshComponent* Mesh : Meshes)
		{
			if (Mesh->GetAttachParent() == Body && !Mesh->bHiddenInGame && Mesh->GetSkeletalMeshAsset()) return Mesh;
		}
	}
	return Body;
}

bool UCPGA_PlayerEnemyExplosion::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;
	ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	const ACPEnemyCharacter* Enemy = Cast<ACPEnemyCharacter>(Character);
	const UCPHealthComponent* Health = Character ? Character->FindComponentByClass<UCPHealthComponent>() : nullptr;
	USkeletalMeshComponent* Mesh = FindVisualMesh(Character);
	return IsValid(Character) && (Enemy || (Character->IsPlayerControlled() && Health)) && !(Enemy && Enemy->IsDead()) && !(Health && Health->IsDead()) && Mesh && Mesh->GetPhysicsAsset() && !Mesh->IsSimulatingPhysics() && PullDuration > 0.0f;
}

void UCPGA_PlayerEnemyExplosion::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	const FHitResult* Hit = TriggerEventData ? TriggerEventData->ContextHandle.GetHitResult() : nullptr;
	USkeletalMeshComponent* Mesh = FindVisualMesh(Character);
	if (!Character || !Mesh || !Hit || !GetWorld())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}
	CachedCharacter = Character;
	CachedMesh = Mesh;
	CachedController = Character->GetController();
	CachedHealth = Character->FindComponentByClass<UCPHealthComponent>();
	DamageInstigator = TriggerEventData->ContextHandle.GetInstigator();
	Center = Hit->ImpactPoint + FVector(0.0f, 0.0f, CenterZOffset);
	OriginalActorLocation = Character->GetActorLocation();
	OriginalParent = Mesh->GetAttachParent();
	OriginalSocket = Mesh->GetAttachSocketName();
	OriginalRelativeTransform = Mesh->GetRelativeTransform();
	OriginalMeshProfile = Mesh->GetCollisionProfileName();
	OriginalMeshCollision = Mesh->GetCollisionEnabled();
	bOriginalGravity = Mesh->IsGravityEnabled();
	OriginalCapsuleCollision = Character->GetCapsuleComponent()->GetCollisionEnabled();
	OriginalMovementMode = Character->GetCharacterMovement()->MovementMode;
	OriginalCustomMovementMode = Character->GetCharacterMovement()->CustomMovementMode;
	bOriginalCCD = Mesh->BodyInstance.bUseCCD;
	bOriginalNotifyHit = Mesh->BodyInstance.bNotifyRigidBodyCollision;
	bPrepared = true;
	if (AAIController* AI = Cast<AAIController>(CachedController.Get()))
	{
		AI->StopMovement();
		CachedBrain = AI->GetBrainComponent();
		if (CachedBrain.IsValid() && CachedBrain->IsRunning() && !CachedBrain->IsPaused())
		{
			CachedBrain->PauseLogic(TEXT("Explosion"));
			bPausedBrain = true;
		}
	}
	if (APlayerController* PC = Cast<APlayerController>(CachedController.Get()))
	{
		PC->SetIgnoreMoveInput(true);
		bBlockedInput = true;
	}
	CachedInteraction = Character->FindComponentByClass<UCPInteractionComponent>();
	bStartedAsSlime = GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Slime")));
	if (CachedInteraction.IsValid())
	{
		bOriginalInteractionBlocked = CachedInteraction->IsInteractionBlocked();
		CachedInteraction->SetInteractionBlocked(true);
	}
	if (ACPEnemyCharacter* Enemy = Cast<ACPEnemyCharacter>(Character))
	{
		Enemy->CancelAttack();
		DyingHandle = Enemy->OnEnemyDying.AddUObject(this, &UCPGA_PlayerEnemyExplosion::FinishReaction);
	}
	if (CachedHealth.IsValid()) CachedHealth->OnDeath.AddUniqueDynamic(this, &UCPGA_PlayerEnemyExplosion::HandleDeath);
	Character->OnEndPlay.AddUniqueDynamic(this, &UCPGA_PlayerEnemyExplosion::HandleEndPlay);
	Character->GetCharacterMovement()->StopMovementImmediately();
	Character->GetCharacterMovement()->DisableMovement();
	Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
	// 캡슐·카메라 이동이 물리 바디를 다시 이동시키지 않도록 분리
	Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetAllBodiesSimulatePhysics(true);
	Mesh->SetEnableGravity(false);
	Mesh->SetUseCCD(true);
	Mesh->SetNotifyRigidBodyCollision(true);
	Mesh->OnComponentHit.AddUniqueDynamic(this, &UCPGA_PlayerEnemyExplosion::HandleMeshHit);
	Mesh->WakeAllRigidBodies();
	CoreBone = NAME_None;
	float LargestMass = -1.0f;
	for (FBodyInstance* Body : Mesh->Bodies)
	{
		if (Body && Body->IsValidBodyInstance() && Body->bSimulatePhysics && Body->GetBodyMass() > LargestMass)
		{
			LargestMass = Body->GetBodyMass();
			CoreBone = Mesh->GetBoneName(Body->InstanceBoneIndex);
		}
	}
	if (LargestMass < 0.0f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}
	FVector Position, Velocity;
	float Mass;
	GetBodyState(Position, Velocity, Mass);
	StartRadius = FVector::Dist2D(Position, Center);
	ActualOrbitRadius = OrbitRadius * FMath::FRandRange(1.0f - OrbitRadiusJitter, 1.0f + OrbitRadiusJitter);
	Phase = EPhase::Waiting;
	PhaseStartTime = LastTickTime = GetWorld()->GetTimeSeconds();
	TurbulenceElapsed = 0.0f;
	LastBounceTime = -1.0;
	if (StartSFX) UGameplayStatics::PlaySoundAtLocation(this, StartSFX, Position);
	GetWorld()->GetTimerManager().SetTimer(ReactionTimer, this, &UCPGA_PlayerEnemyExplosion::UpdateReaction, 0.016f, true);
}

void UCPGA_PlayerEnemyExplosion::GetBodyState(FVector& Position, FVector& Velocity, float& Mass) const
{
	Position = Velocity = FVector::ZeroVector;
	Mass = 0.0f;
	if (USkeletalMeshComponent* Mesh = CachedMesh.Get())
	{
		for (FBodyInstance* Body : Mesh->Bodies)
		{
			if (!Body || !Body->IsValidBodyInstance() || !Body->bSimulatePhysics) continue;
			const float BodyMass = Body->GetBodyMass();
			Position += Body->GetCOMPosition() * BodyMass;
			Velocity += Body->GetUnrealWorldVelocity() * BodyMass;
			Mass += BodyMass;
		}
		if (Mass > KINDA_SMALL_NUMBER) { Position /= Mass; Velocity /= Mass; }
		else { Position = Mesh->Bounds.Origin; Mass = 1.0f; }
	}
}

void UCPGA_PlayerEnemyExplosion::UpdateReaction()
{
	ACharacter* Character = CachedCharacter.Get();
	USkeletalMeshComponent* Mesh = CachedMesh.Get();
	if (!Character || !Mesh || !Mesh->IsSimulatingPhysics()) { FinishReaction(); return; }
	const double Now = GetWorld()->GetTimeSeconds();
	const float DeltaTime = FMath::Clamp(float(Now - LastTickTime), 0.0f, 0.1f);
	LastTickTime = Now;
	FVector Position, Velocity;
	float Mass;
	GetBodyState(Position, Velocity, Mass);
	// 카메라와 상태 판정의 기준 액터를 물리 외형의 위치로 이동
	Character->SetActorLocation(Position + FVector::UpVector * Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), false, nullptr, ETeleportType::TeleportPhysics);
	if (Phase == EPhase::Waiting && Now - PhaseStartTime >= PullDelay)
	{
		Phase = EPhase::Pulling;
		PhaseStartTime = Now;
		ExecuteSharedCue(GetAbilitySystemComponentFromActorInfo(), GetWorld(), GatherCueTag, Center, OrbitRadius, GatherSlot);
	}
	if (Phase == EPhase::Pulling)
	{
		if (Now - PhaseStartTime >= PullDuration) Blast();
		else ApplyPull(DeltaTime);
	}
	else if (Phase == EPhase::Blasted && Now - PhaseStartTime >= BounceDuration)
	{
		if (Velocity.SizeSquared() <= FMath::Square(20.0f) || Now - PhaseStartTime >= BounceDuration + MaxSettleTime) FinishReaction();
	}
}

void UCPGA_PlayerEnemyExplosion::ApplyPull(float DeltaTime)
{
	USkeletalMeshComponent* Mesh = CachedMesh.Get();
	FVector Position, Velocity;
	float Mass;
	GetBodyState(Position, Velocity, Mass);
	const FVector Relative(Position.X - Center.X, Position.Y - Center.Y, 0.0f);
	const float Radius = FMath::Max(Relative.Size(), 30.0f);
	const FVector Radial = Relative.IsNearlyZero() ? FVector::ForwardVector : Relative.GetSafeNormal();
	const float Omega = FMath::DegreesToRadians(OrbitSpeedDegPerSec);
	const FVector Tangent = FVector::CrossProduct(FVector::UpVector, Radial) * (Omega >= 0.0f ? 1.0f : -1.0f);
	const float Progress = StartRadius > ActualOrbitRadius + KINDA_SMALL_NUMBER ? 1.0f - FMath::Clamp((Radius - ActualOrbitRadius) / (StartRadius - ActualOrbitRadius), 0.0f, 1.0f) : 1.0f;
	const float AngularSpeed = FMath::Min3(FMath::Abs(Omega), MaxPullSpeed / Radius, FMath::Sqrt(MaxPullAccel * 0.6f / Radius));
	const float TangentialSpeed = FVector::DotProduct(Velocity, Tangent);
	const float RadialTarget = FMath::Clamp((ActualOrbitRadius - Radius) * PullStiffness, -MaxPullSpeed, MaxPullSpeed);
	const float TangentTarget = AngularSpeed * FMath::Lerp(0.25f, 1.0f, Progress) * Radius;
	const float VerticalTarget = FMath::Clamp((Center.Z - Position.Z) * PullStiffness, -MaxPullSpeed, MaxPullSpeed);
	const FVector Acceleration = (Radial * ((RadialTarget - FVector::DotProduct(Velocity, Radial)) * WindFollowGain) + Tangent * ((TangentTarget - TangentialSpeed) * WindFollowGain) + FVector::UpVector * ((VerticalTarget - Velocity.Z) * WindFollowGain) - Radial * (TangentialSpeed * TangentialSpeed / Radius)).GetClampedToMaxSize(MaxPullAccel);
	const FVector DeltaVelocity = Acceleration * DeltaTime;
	for (FBodyInstance* Body : Mesh->Bodies)
	{
		if (Body && Body->IsValidBodyInstance() && Body->bSimulatePhysics) Body->AddImpulse(DeltaVelocity * (1.0f - CoreDriveRatio), true);
	}
	if (FBodyInstance* Core = Mesh->GetBodyInstance(CoreBone))
	{
		Core->AddImpulse(DeltaVelocity * Mass * CoreDriveRatio, false);
		TurbulenceElapsed += DeltaTime;
		if (TurbulenceElapsed >= 0.15f)
		{
			Core->AddAngularImpulseInRadians(FMath::VRand() * FMath::DegreesToRadians(TurbulenceSpin) * Progress, true);
			TurbulenceElapsed = 0.0f;
		}
	}
}

void UCPGA_PlayerEnemyExplosion::Blast()
{
	Phase = EPhase::Blasted;
	PhaseStartTime = GetWorld()->GetTimeSeconds();
	ExecuteSharedCue(GetAbilitySystemComponentFromActorInfo(), GetWorld(), ExplodeCueTag, Center, OrbitRadius, ExplodeSlot);
	USkeletalMeshComponent* Mesh = CachedMesh.Get();
	FVector Position, Velocity;
	float Mass;
	GetBodyState(Position, Velocity, Mass);
	FVector Direction = (Position - Center).GetSafeNormal();
	if (Direction.IsNearlyZero()) Direction = FMath::VRand();
	const FVector BlastVelocity = (Direction + FVector::UpVector * ExplosionUpwardBias).GetSafeNormal() * ExplosionSpeed * FMath::FRandRange(0.85f, 1.15f);
	Mesh->SetEnableGravity(true);
	Mesh->SetAllPhysicsLinearVelocity(BlastVelocity * LimbSpeedRatio);
	if (FBodyInstance* Core = Mesh->GetBodyInstance(CoreBone)) Core->SetLinearVelocity(BlastVelocity, false);
	Mesh->AddAngularImpulseInDegrees(FMath::VRand() * ExplosionSpin, NAME_None, true);
	// 단계를 먼저 전환하여 사망·취소 콜백에서도 피해 중복 방지
	if (CachedHealth.IsValid() && !Cast<ACPEnemyCharacter>(CachedCharacter.Get())) CachedHealth->ApplyDamage(PlayerDamage, DamageInstigator.Get());
}

void UCPGA_PlayerEnemyExplosion::HandleMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (Phase != EPhase::Blasted || !CachedMesh.IsValid() || !GetWorld()) return;
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - PhaseStartTime > BounceDuration || Now - LastBounceTime < 0.1) return;
	LastBounceTime = Now;
	USkeletalMeshComponent* Mesh = CachedMesh.Get();
	const FVector Velocity = Mesh->GetComponentVelocity();
	if (Velocity.IsNearlyZero()) return;
	const FVector Direction = (FMath::GetReflectionVector(Velocity.GetSafeNormal(), Hit.ImpactNormal) + Hit.ImpactNormal * 0.5f).GetSafeNormal();
	Mesh->SetPhysicsLinearVelocity(Direction * FMath::Max(Velocity.Size(), BounceMinSpeed));
	Mesh->AddAngularImpulseInDegrees(FMath::VRand() * BounceSpin, NAME_None, true);
}

void UCPGA_PlayerEnemyExplosion::FinishReaction()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UCPGA_PlayerEnemyExplosion::HandleDeath(AActor* DamageCauser) { FinishReaction(); }
void UCPGA_PlayerEnemyExplosion::HandleEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason) { FinishReaction(); }

void UCPGA_PlayerEnemyExplosion::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(ReactionTimer);
	if (bPrepared)
	{
		bPrepared = false;
		ACharacter* Character = CachedCharacter.Get();
		USkeletalMeshComponent* Mesh = CachedMesh.Get();
		if (Character)
		{
			Character->OnEndPlay.RemoveDynamic(this, &UCPGA_PlayerEnemyExplosion::HandleEndPlay);
			ACPEnemyCharacter* Enemy = Cast<ACPEnemyCharacter>(Character);
			if (Enemy) Enemy->OnEnemyDying.Remove(DyingHandle);
			const bool bDead = (Enemy && Enemy->IsDead()) || (CachedHealth.IsValid() && CachedHealth->IsDead());
			if (Mesh)
			{
				FVector Position, Velocity;
				float Mass;
				GetBodyState(Position, Velocity, Mass);
				Mesh->OnComponentHit.RemoveDynamic(this, &UCPGA_PlayerEnemyExplosion::HandleMeshHit);
				Mesh->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
				Mesh->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
				Mesh->SetAllBodiesSimulatePhysics(false);
				Mesh->SetSimulatePhysics(false);
				Mesh->SetEnableGravity(bOriginalGravity);
				Mesh->SetUseCCD(bOriginalCCD);
				Mesh->SetNotifyRigidBodyCollision(bOriginalNotifyHit);
				Mesh->SetCollisionProfileName(OriginalMeshProfile);
				Mesh->SetCollisionEnabled(OriginalMeshCollision);
				FHitResult Floor;
				FCollisionQueryParams Query;
				Query.AddIgnoredActor(Character);
				FVector RecoveryLocation = Position + FVector::UpVector * Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
				if (GetWorld()->LineTraceSingleByChannel(Floor, Position + FVector::UpVector * 100.0f, Position - FVector::UpVector * 3000.0f, ECC_WorldStatic, Query)) RecoveryLocation = Floor.ImpactPoint + FVector::UpVector * (Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.0f);
				Character->GetCapsuleComponent()->SetCollisionEnabled(OriginalCapsuleCollision);
				if (!GetWorld()->FindTeleportSpot(Character, RecoveryLocation, Character->GetActorRotation())) RecoveryLocation = OriginalActorLocation;
				Character->SetActorLocation(RecoveryLocation, false, nullptr, ETeleportType::TeleportPhysics);
				Mesh->AttachToComponent(OriginalParent.Get() ? OriginalParent.Get() : Character->GetCapsuleComponent(), FAttachmentTransformRules::KeepWorldTransform, OriginalSocket);
				// 도중에 Enemy Giant가 종료되어도 현재 부모 크기에 맞춰 복구
				Mesh->SetRelativeTransform(OriginalRelativeTransform, false, nullptr, ETeleportType::TeleportPhysics);
				Mesh->RefreshBoneTransforms();
			}
			else Character->GetCapsuleComponent()->SetCollisionEnabled(OriginalCapsuleCollision);
			if (!bDead)
			{
				Character->GetCharacterMovement()->SetMovementMode(OriginalMovementMode, OriginalCustomMovementMode);
				if (bPausedBrain && CachedBrain.IsValid()) CachedBrain->ResumeLogic(TEXT("Explosion complete"));
			}
			if (!bWasCancelled && !bDead && EndSFX) UGameplayStatics::PlaySoundAtLocation(this, EndSFX, Character->GetActorLocation());
		}
		if (bBlockedInput && CachedController.IsValid()) CachedController->SetIgnoreMoveInput(false);
		if (CachedInteraction.IsValid())
		{
			// 도중에 Slime이 만료되었다면 변신 당시의 상호작용 차단을 복원하지 않는다.
			const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
			const bool bSlimeExpired = bStartedAsSlime && ASC && !ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Slime")));
			CachedInteraction->SetInteractionBlocked(bOriginalInteractionBlocked && !bSlimeExpired);
		}
		if (CachedHealth.IsValid()) CachedHealth->OnDeath.RemoveDynamic(this, &UCPGA_PlayerEnemyExplosion::HandleDeath);
	}
	bPausedBrain = bBlockedInput = false;
	CachedCharacter.Reset(); CachedMesh.Reset(); CachedBrain.Reset(); CachedController.Reset(); CachedHealth.Reset(); CachedInteraction.Reset();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, false, bWasCancelled);
}
