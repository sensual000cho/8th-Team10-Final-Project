#include "Character/Enemy/GA/CPGA_EnemySlime.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Character/Enemy/CPEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogCPEnemySlime, Log, All);

UCPGA_EnemySlime::UCPGA_EnemySlime()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer InitialAssetTags;
	InitialAssetTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Reaction.Potion")));
	SetAssetTags(InitialAssetTags);
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.All")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Immunity.Potion.Slime")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Reaction.Potion")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Reaction.Potion")));

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Potion.Effect.Slime"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

bool UCPGA_EnemySlime::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const ACPEnemyCharacter* Enemy = ActorInfo ? Cast<ACPEnemyCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	return IsValid(Enemy) && !Enemy->IsDead() && Enemy->GetSlimeSplitCount() < MaxSplitCount && Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UCPGA_EnemySlime::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACPEnemyCharacter* Enemy = ActorInfo ? Cast<ACPEnemyCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UWorld* World = GetWorld();
	if (!IsValid(Enemy) || Enemy->IsDead() || Enemy->GetSlimeSplitCount() >= MaxSplitCount || !World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	FVector OriginalLocation;
	FVector CloneLocation;
	if (!FindSplitLocations(*Enemy, OriginalLocation, CloneLocation))
	{
		UE_LOG(LogCPEnemySlime, Warning, TEXT("%s: Split cancelled; no safe pair of positions (ground, clearance or wall check). Location=%s"), *Enemy->GetName(), *Enemy->GetActorLocation().ToCompactString());
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// 복제본을 먼저 생성하므로 Spawn 실패 시 원본은 전혀 바뀌지 않는다.
	const FTransform CloneTransform(Enemy->GetActorRotation(), CloneLocation, Enemy->GetActorScale3D() * SplitScaleMultiplier);
	FActorSpawnParameters SpawnParams;
	SpawnParams.OverrideLevel = Enemy->GetLevel();
	// 기본 크기의 사전 검사 대신 실제 축소된 크기로 충돌 검사와 위치 보정을 받는다.
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	ACPEnemyCharacter* Clone = World->SpawnActor<ACPEnemyCharacter>(Enemy->GetClass(), CloneTransform, SpawnParams);
	if (!IsValid(Clone))
	{
		UE_LOG(LogCPEnemySlime, Warning, TEXT("%s: Clone spawn failed. Class=%s Location=%s"), *Enemy->GetName(), *Enemy->GetClass()->GetName(), *CloneLocation.ToCompactString());
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// Spawn된 Class의 초기 AI를 사용하고 원본의 진행 중인 공격/승리 상태는 복사하지 않는다.
	Clone->InitializeSlimeSplit(*Enemy, SplitScaleMultiplier);
	Clone->SetLifeSpan(0.0f);
	Enemy->InitializeSlimeSplit(*Enemy, SplitScaleMultiplier);
	Enemy->SetActorLocation(OriginalLocation, false, nullptr, ETeleportType::TeleportPhysics);
	Enemy->CancelAttack();
	if (AAIController* Controller = Cast<AAIController>(Enemy->GetController()))
	{
		Controller->StopMovement();
	}

	const FGameplayTag SlimeStateTag = FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Slime"));
	Enemy->GetAbilitySystemComponent()->SetLooseGameplayTagCount(SlimeStateTag, 1);
	Clone->GetAbilitySystemComponent()->SetLooseGameplayTagCount(SlimeStateTag, 1);

	const FVector SplitDirection = (CloneLocation - OriginalLocation).GetSafeNormal2D();
	// 이미 실행 중인 FartLaunch의 비행 속도는 분열 연출로 덮어쓰지 않는다.
	if (Enemy->GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag(TEXT("State.Reaction.Potion"))) == 1)
	{
		Enemy->LaunchCharacter(-SplitDirection * SplitHorizontalSpeed + FVector::UpVector * SplitUpwardSpeed, true, true);
	}
	Clone->LaunchCharacter(SplitDirection * SplitHorizontalSpeed + FVector::UpVector * SplitUpwardSpeed, true, true);

	// Spawner 등록과 Quest/처치 집계 정책은 여기에서 연결하지 않는다.
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

bool UCPGA_EnemySlime::FindSplitLocations(const ACPEnemyCharacter& Enemy, FVector& OutOriginalLocation, FVector& OutCloneLocation) const
{
	const UCapsuleComponent* Capsule = Enemy.GetCapsuleComponent();
	const float Radius = Capsule->GetScaledCapsuleRadius();
	// 생성 전 원본 Capsule과도 겹치지 않는 거리를 확보한다.
	const float Offset = Radius * (1.0f + SplitScaleMultiplier) + SplitSpacing;
	const FVector Center = Enemy.GetActorLocation();
	const FVector Right = Enemy.GetActorRightVector().GetSafeNormal2D();
	const float CandidateAngles[] = { 0.0f, 30.0f, -30.0f, 60.0f, -60.0f, 90.0f };

	for (const float Angle : CandidateAngles)
	{
		const FVector Direction = Right.RotateAngleAxis(Angle, FVector::UpVector);
		if (FindGroundedLocation(Enemy, Center - Direction * Offset, OutOriginalLocation) && FindGroundedLocation(Enemy, Center + Direction * Offset, OutCloneLocation))
		{
			return true;
		}
	}
	return false;
}

bool UCPGA_EnemySlime::FindGroundedLocation(const ACPEnemyCharacter& Enemy, const FVector& Candidate, FVector& OutLocation) const
{
	const UCapsuleComponent* Capsule = Enemy.GetCapsuleComponent();
	const float Radius = Capsule->GetScaledCapsuleRadius() * SplitScaleMultiplier;
	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight() * SplitScaleMultiplier;

	// NavMesh와 무관하게 발밑 주변의 실제 바닥을 검사하고 좌우 위치는 유지한다.
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemySlimeSplit), false, &Enemy);
	const FName Profile = Capsule->GetCollisionProfileName();
	const FVector GroundProbe = Candidate - FVector::UpVector * Capsule->GetScaledCapsuleHalfHeight();
	const FCollisionShape Shape = FCollisionShape::MakeCapsule(Radius, HalfHeight);
	FHitResult GroundHit;
	if (!GetWorld()->SweepSingleByProfile(GroundHit, GroundProbe + FVector::UpVector * (HalfHeight + 50.0f), GroundProbe + FVector::UpVector * (HalfHeight - 200.0f), FQuat::Identity, Profile, Shape, QueryParams) || GroundHit.bStartPenetrating || !Enemy.GetCharacterMovement()->IsWalkable(GroundHit))
	{
		return false;
	}

	OutLocation = GroundHit.Location + FVector::UpVector * 2.0f;
	if (GetWorld()->OverlapBlockingTestByProfile(OutLocation, FQuat::Identity, Profile, Shape, QueryParams))
	{
		return false;
	}

	// 벽 너머로 분열하지 않도록 중심에서 목적지까지 축소 Capsule로 검사한다.
	FHitResult PathHit;
	return !GetWorld()->SweepSingleByProfile(PathHit, Enemy.GetActorLocation(), OutLocation, FQuat::Identity, Profile, Shape, QueryParams);
}
