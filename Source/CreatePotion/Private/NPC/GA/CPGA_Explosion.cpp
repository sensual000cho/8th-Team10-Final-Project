#include "NPC/GA/CPGA_Explosion.h"
#include "NPC/CPBaseNPC.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "PhysicsEngine/BodyInstance.h"
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

	void ExecuteSharedCue(UAbilitySystemComponent* ASC, const UWorld* World, const FGameplayTag& Tag,
		const FVector& Center, float Radius, FSharedCueSlot& Slot)
	{
		if (!ASC || !World || !Tag.IsValid() || !TryClaimSharedCue(World, Center, Slot)) return;

		FGameplayCueParameters Params;
		Params.Location = Center;
		Params.RawMagnitude = Radius;
		ASC->ExecuteGameplayCue(Tag, Params);
	}

	bool IsSimulatingBody(const FBodyInstance* Body)
	{
		return Body && Body->bSimulatePhysics && Body->IsValidBodyInstance();
	}
}

UCPGA_Explosion::UCPGA_Explosion()
{
	ImmunityTag = FGameplayTag::RequestGameplayTag(FName("Immunity.Potion.Explosion"));
}

void UCPGA_Explosion::GetBodyState(USkeletalMeshComponent* Mesh, FVector& OutPos, FVector& OutVel, float& OutMass) const
{
	OutPos = FVector::ZeroVector;
	OutVel = FVector::ZeroVector;
	OutMass = 0.0f;

	for (FBodyInstance* Body : Mesh->Bodies)
	{
		if (!IsSimulatingBody(Body)) continue;

		const float Mass = Body->GetBodyMass();
		OutPos += Body->GetCOMPosition() * Mass;
		OutVel += Body->GetUnrealWorldVelocity() * Mass;
		OutMass += Mass;
	}

	if (OutMass > KINDA_SMALL_NUMBER)
	{
		OutPos /= OutMass;
		OutVel /= OutMass;
	}
	else
	{
		OutPos = Mesh->Bounds.Origin;
		OutVel = Mesh->GetComponentVelocity();
		OutMass = 1.0f;
	}
}

FVector UCPGA_Explosion::GetCorePosition(USkeletalMeshComponent* Mesh) const
{
	FVector Pos, Vel;
	float Mass;
	GetBodyState(Mesh, Pos, Vel, Mass);
	return Pos;
}

FBodyInstance* UCPGA_Explosion::GetCoreBody(USkeletalMeshComponent* Mesh) const
{
	if (!Mesh || !Mesh->Bodies.IsValidIndex(CoreBodyIndex)) return nullptr;

	FBodyInstance* Body = Mesh->Bodies[CoreBodyIndex];
	return IsSimulatingBody(Body) ? Body : nullptr;
}

void UCPGA_Explosion::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACPBaseNPC* NPC = PrepareReactionNPC(Handle, ActorInfo, ActivationInfo);
	if (!NPC) return;

	FHitResult Hit;
	UWorld* World = GetWorld();
	if (!ExtractImpactHitResult(TriggerEventData, Hit) || !World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	EnterRagdoll(NPC);
	bReactToMeshBounce = false;

	USkeletalMeshComponent* Mesh = NPC->GetMesh();
	if (!Mesh)
	{
		ForceEndAbility();
		return;
	}

	Center = Hit.ImpactPoint + FVector(0.0f, 0.0f, CenterZOffset);

	CoreBodyIndex = INDEX_NONE;
	float MaxMass = -1.0f;
	for (int32 i = 0; i < Mesh->Bodies.Num(); ++i)
	{
		const FBodyInstance* Body = Mesh->Bodies[i];
		if (IsSimulatingBody(Body) && Body->GetBodyMass() > MaxMass)
		{
			MaxMass = Body->GetBodyMass();
			CoreBodyIndex = i;
		}
	}

	const FVector Core = GetCorePosition(Mesh);
	StartRadius = FVector2D(Core.X - Center.X, Core.Y - Center.Y).Size();
	ActualOrbitRadius = OrbitRadius * FMath::FRandRange(1.0f - OrbitRadiusJitter, 1.0f + OrbitRadiusJitter);
	PullStartTime = LastTickTime = World->GetTimeSeconds();
	TurbulenceTimer = 0.0f;

	if (PullDelay > 0.0f)
	{
		World->GetTimerManager().SetTimer(PullTimerHandle, this, &UCPGA_Explosion::StartPull, PullDelay, false);
	}
	else
	{
		StartPull();
	}
}

void UCPGA_Explosion::StartPull()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		ForceEndAbility();
		return;
	}

	ExecuteSharedCue(GetAbilitySystemComponentFromActorInfo(), World, GatherCueTag, Center, OrbitRadius, GatherSlot);

	PullStartTime = LastTickTime = World->GetTimeSeconds();
	World->GetTimerManager().SetTimer(PullTimerHandle, this, &UCPGA_Explosion::TickPull, 0.016f, true);
}

void UCPGA_Explosion::TickPull()
{
	USkeletalMeshComponent* Mesh = GetRagdollingMesh();
	UWorld* World = GetWorld();
	if (!Mesh || !World)
	{
		ForceEndAbility();
		return;
	}

	const float Now = World->GetTimeSeconds();
	const float Dt = FMath::Max(Now - LastTickTime, KINDA_SMALL_NUMBER);
	LastTickTime = Now;

	if (Now - PullStartTime >= PullDuration)
	{
		Explode(Mesh);
		return;
	}

	FVector Core, CoreVel;
	float TotalMass;
	GetBodyState(Mesh, Core, CoreVel, TotalMass);
	const FVector CoreRel(Core.X - Center.X, Core.Y - Center.Y, 0.0f);
	const float CoreRadius = FMath::Max(CoreRel.Size(), 30.0f);

	float DistanceAlpha = 1.0f;
	if (StartRadius > ActualOrbitRadius + KINDA_SMALL_NUMBER)
	{
		DistanceAlpha = 1.0f - FMath::Clamp((CoreRadius - ActualOrbitRadius) / (StartRadius - ActualOrbitRadius), 0.0f, 1.0f);
	}

	const FVector Radial = CoreRel.IsNearlyZero() ? FVector::ForwardVector : CoreRel.GetSafeNormal();

	const float Omega = FMath::DegreesToRadians(OrbitSpeedDegPerSec);
	const FVector Tangent = FVector::CrossProduct(FVector::UpVector, Radial) * (Omega >= 0.0f ? 1.0f : -1.0f);
	const float AbsOmega = FMath::Min3(FMath::Abs(Omega), MaxPullSpeed / CoreRadius, FMath::Sqrt(MaxPullAccel * 0.6f / CoreRadius));

	const float VRadial = FVector::DotProduct(CoreVel, Radial);
	const float VTangent = FVector::DotProduct(CoreVel, Tangent);
	const float VRadialTarget = FMath::Clamp((ActualOrbitRadius - CoreRadius) * PullStiffness, -MaxPullSpeed, MaxPullSpeed);
	const float VTangentTarget = AbsOmega * FMath::Lerp(0.25f, 1.0f, DistanceAlpha) * CoreRadius;
	const float VZTarget = FMath::Clamp((Center.Z - Core.Z) * PullStiffness, -MaxPullSpeed, MaxPullSpeed);

	FVector Accel = Radial * ((VRadialTarget - VRadial) * WindFollowGain)
		+ Tangent * ((VTangentTarget - VTangent) * WindFollowGain)
		+ FVector::UpVector * ((VZTarget - CoreVel.Z) * WindFollowGain)
		- Radial * (VTangent * VTangent / CoreRadius);
	Accel = Accel.GetClampedToMaxSize(MaxPullAccel);
	const FVector DeltaV = Accel * Dt;

	TurbulenceTimer += Dt;
	const bool bApplyTurbulence = TurbulenceTimer >= TurbulenceInterval;
	FVector Spin = FVector::ZeroVector;
	if (bApplyTurbulence)
	{
		TurbulenceTimer = 0.0f;
		Spin = FMath::VRand() * FMath::DegreesToRadians(TurbulenceSpinDegPerSec) * DistanceAlpha;
	}

	const float RestRatio = 1.0f - CoreDriveRatio;
	if (RestRatio > KINDA_SMALL_NUMBER)
	{
		for (FBodyInstance* Body : Mesh->Bodies)
		{
			if (IsSimulatingBody(Body)) Body->AddImpulse(DeltaV * RestRatio, true);
		}
	}

	const FVector CoreImpulse = DeltaV * TotalMass * CoreDriveRatio;
	if (FBodyInstance* CoreBody = GetCoreBody(Mesh))
	{
		CoreBody->AddImpulse(CoreImpulse, false);
		if (bApplyTurbulence) CoreBody->AddAngularImpulseInRadians(Spin, true);
	}
	else
	{
		Mesh->AddImpulse(CoreImpulse, NAME_None, false);
		if (bApplyTurbulence) Mesh->AddAngularImpulseInRadians(Spin, NAME_None, true);
	}
}

void UCPGA_Explosion::Explode(USkeletalMeshComponent* Mesh)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		ForceEndAbility();
		return;
	}

	FTimerManager& TM = World->GetTimerManager();
	TM.ClearTimer(PullTimerHandle);

	ExecuteSharedCue(GetAbilitySystemComponentFromActorInfo(), World, ExplodeCueTag, Center, OrbitRadius, ExplodeSlot);

	FVector Dir = (GetCorePosition(Mesh) - Center).GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		Dir = FMath::VRand();
	}
	Dir = (Dir + FVector::UpVector * ExplosionUpwardBias).GetSafeNormal();
	const FVector Blast = Dir * ExplosionSpeed * FMath::FRandRange(0.85f, 1.15f);

	Mesh->SetEnableGravity(true);
	Mesh->SetAllPhysicsLinearVelocity(Blast * ExplosionLimbSpeedRatio);
	if (FBodyInstance* CoreBody = GetCoreBody(Mesh))
	{
		CoreBody->SetLinearVelocity(Blast, false);
	}
	else
	{
		Mesh->SetPhysicsLinearVelocity(Blast);
	}
	Mesh->AddAngularImpulseInDegrees(FMath::VRand() * ExplosionSpin, NAME_None, true);

	bReactToMeshBounce = true;
	TM.SetTimer(ExplosionTimerHandle, this, &UCPGA_Explosion::FinishExplosionForce, ExplosionBounceDuration, false);
}

void UCPGA_Explosion::FinishExplosionForce()
{
	TransitionToNaturalSettling(ExplosionTimerHandle);
}

void UCPGA_Explosion::CleanupNPCReaction(const FGameplayAbilityActorInfo* ActorInfo)
{
	ClearTimers({ &PullTimerHandle, &ExplosionTimerHandle });
	CoreBodyIndex = INDEX_NONE;
	Super::CleanupNPCReaction(ActorInfo);
}