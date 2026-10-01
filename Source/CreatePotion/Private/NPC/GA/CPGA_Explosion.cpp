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
		FVector Location = FVector::ZeroVector;
		double Time = -1000.0;
	};

	bool TryClaimSharedCue(const UWorld* World, const FVector& Loc, FSharedCueSlot& Slot)
	{
		const double Now = World->GetTimeSeconds();
		if (Now - Slot.Time < 0.3 && FVector::DistSquared(Loc, Slot.Location) < 1.0)
		{
			return false;
		}
		Slot.Time = Now;
		Slot.Location = Loc;
		return true;
	}

	FSharedCueSlot GatherSlot;
	FSharedCueSlot ExplodeSlot;
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
		if (!Body || !Body->bSimulatePhysics || !Body->IsValidBodyInstance()) continue;

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

	CoreBody = nullptr;
	float MaxMass = -1.0f;
	for (FBodyInstance* Body : Mesh->Bodies)
	{
		if (Body && Body->bSimulatePhysics && Body->IsValidBodyInstance() && Body->GetBodyMass() > MaxMass)
		{
			MaxMass = Body->GetBodyMass();
			CoreBody = Body;
		}
	}

	const FVector Core = GetCorePosition(Mesh);
	StartRadius = FVector2D(Core.X - Center.X, Core.Y - Center.Y).Size();
	StartZ = Core.Z;
	ActualOrbitRadius = OrbitRadius * FMath::FRandRange(1.0f - OrbitRadiusJitter, 1.0f + OrbitRadiusJitter);
	PullStartTime = LastTickTime = World->GetTimeSeconds();
	TurbulenceTimer = 0.0f;

	auto PlayGatherCue = [this, ActorInfo, World]()
		{
			if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
			{
				if (GatherCueTag.IsValid() && TryClaimSharedCue(World, Center, GatherSlot))
				{
					FGameplayCueParameters CueParams;
					CueParams.Location = Center;
					CueParams.RawMagnitude = OrbitRadius;
					ASC->ExecuteGameplayCue(GatherCueTag, CueParams);
				}
			}
		};

	if (PullDelay > 0.0f)
	{
		World->GetTimerManager().SetTimer(PullTimerHandle, [this, World, PlayGatherCue]()
			{
				if (!IsValid(this)) return;
				PlayGatherCue();

				PullStartTime = LastTickTime = World->GetTimeSeconds();
				World->GetTimerManager().SetTimer(PullTimerHandle, this, &UCPGA_Explosion::TickPull, 0.016f, true);
			}, PullDelay, false);
	}
	else
	{
		PlayGatherCue();
		World->GetTimerManager().SetTimer(PullTimerHandle, this, &UCPGA_Explosion::TickPull, 0.016f, true);
	}
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
	const float Elapsed = Now - PullStartTime;

	if (Elapsed >= PullDuration)
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

	const float TargetRadius = ActualOrbitRadius;
	const float TargetZ = Center.Z;

	const FVector Radial = CoreRel.IsNearlyZero() ? FVector::ForwardVector : CoreRel.GetSafeNormal();

	const float Omega = FMath::DegreesToRadians(OrbitSpeedDegPerSec);
	const FVector Tangent = FVector::CrossProduct(FVector::UpVector, Radial) * (Omega >= 0.0f ? 1.0f : -1.0f);

	const float AbsOmega = FMath::Min3(FMath::Abs(Omega), MaxPullSpeed / CoreRadius, FMath::Sqrt(MaxPullAccel * 0.6f / CoreRadius));

	const float VRadial = FVector::DotProduct(CoreVel, Radial);
	const float VTangent = FVector::DotProduct(CoreVel, Tangent);
	const float VRadialTarget = FMath::Clamp((TargetRadius - CoreRadius) * PullStiffness, -MaxPullSpeed, MaxPullSpeed);

	const float SwirlScale = FMath::Lerp(0.25f, 1.0f, DistanceAlpha);
	const float VTangentTarget = AbsOmega * SwirlScale * CoreRadius;

	const float VZTarget = FMath::Clamp((TargetZ - Core.Z) * PullStiffness, -MaxPullSpeed, MaxPullSpeed);

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
			if (!Body || !Body->bSimulatePhysics || !Body->IsValidBodyInstance()) continue;
			Body->AddImpulse(DeltaV * RestRatio, true);
		}
	}

	const FVector CoreImpulse = DeltaV * TotalMass * CoreDriveRatio;
	if (CoreBody && CoreBody->IsValidBodyInstance())
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
	FTimerManager& TM = World->GetTimerManager();
	TM.ClearTimer(PullTimerHandle);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (ExplodeCueTag.IsValid() && TryClaimSharedCue(World, Center, ExplodeSlot))
		{
			FGameplayCueParameters CueParams;
			CueParams.Location = Center;
			CueParams.RawMagnitude = OrbitRadius;
			ASC->ExecuteGameplayCue(ExplodeCueTag, CueParams);
		}
	}

	FVector Dir = (GetCorePosition(Mesh) - Center).GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		Dir = FMath::VRand();
	}
	Dir = (Dir + FVector::UpVector * ExplosionUpwardBias).GetSafeNormal();
	const FVector Blast = Dir * ExplosionSpeed * FMath::FRandRange(0.85f, 1.15f);

	Mesh->SetEnableGravity(true);
	Mesh->SetAllPhysicsLinearVelocity(Blast * ExplosionLimbSpeedRatio); 
	if (CoreBody && CoreBody->IsValidBodyInstance())
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
	CoreBody = nullptr;
	Super::CleanupNPCReaction(ActorInfo);
}