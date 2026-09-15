// CPEnemySpawner.cpp


#include "Character/Enemy/CPEnemySpawner.h"

#include "Character/Enemy/CPEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "NavigationSystem.h"

ACPEnemySpawner::ACPEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void ACPEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnEnemies();
}

void ACPEnemySpawner::SpawnEnemies()
{
	UWorld* World = GetWorld();
	
	if (!IsValid(World) || !EnemyClass)
	{
		return;
	}
	
	if (!SpawnedEnemies.IsEmpty())
	{
		return;
	}
	
	int32 SuccessCount = 0;
	
	for (int32 EnemyIndex = 0; EnemyIndex < SpawnCount; ++EnemyIndex)
	{
		for (int32 Attempt = 0; Attempt < MaxSpawnAttemptsPerEnemy; ++Attempt)
		{
			FVector SpawnLocation = FVector::ZeroVector;

			if (!TryFindSpawnLocation(SpawnLocation))
			{
				continue;
			}

			ACPEnemyCharacter* Enemy = SpawnEnemyAtLocation(SpawnLocation);

			if (!IsValid(Enemy))
			{
				continue;
			}
			
			SpawnedEnemies.Add(Enemy);
			SpawnedLocations.Add(Enemy->GetActorLocation());

			++SuccessCount;
			break;
		}
	}
}

bool ACPEnemySpawner::TryFindSpawnLocation(FVector& OutLocation)
{
	OutLocation = FVector::ZeroVector;

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	
	if (!IsValid(NavSystem))
	{
		return false;
	}

	FNavLocation NavLocation;

	const bool bFound = NavSystem->GetRandomReachablePointInRadius(
		GetActorLocation(),
		SpawnRadius,
		NavLocation
	);

	if (!bFound)
	{
		return false;
	}

	const FVector CandidateLocation = NavLocation.Location;
	const float MinSpacingSquared = FMath::Square(MinSpawnSpacing);
	
	for (const FVector& ExistingLocation : SpawnedLocations)
	{
		const float DistanceSquared = FVector::DistSquared2D(CandidateLocation, ExistingLocation);

		if (DistanceSquared < MinSpacingSquared)
		{
			return false;
		}
	}
	
	OutLocation = CandidateLocation;
	
	return true;
}

ACPEnemyCharacter* ACPEnemySpawner::SpawnEnemyAtLocation(const FVector& SpawnLocation)
{
	UWorld* World = GetWorld();

	if (!IsValid(World) || !EnemyClass)
	{
		return nullptr;
	}
	
	const ACPEnemyCharacter* DefaultEnemy = EnemyClass->GetDefaultObject<ACPEnemyCharacter>();

	const UCapsuleComponent* DefaultCapsule = IsValid(DefaultEnemy) ? DefaultEnemy->GetCapsuleComponent() : nullptr;

	if (!IsValid(DefaultCapsule))
	{
		return nullptr;
	}
	
	FVector ActorSpawnLocation = SpawnLocation;
	ActorSpawnLocation.Z += DefaultCapsule->GetScaledCapsuleHalfHeight() + SpawnHeightOffset;

	const FRotator SpawnRotation(
		0.f,
		FMath::FRandRange(0.f, 360.f),
		0.f
	);

	FActorSpawnParameters SpawnParams;
	
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
	
	return World->SpawnActor<ACPEnemyCharacter>(
		EnemyClass,
		ActorSpawnLocation,
		SpawnRotation,
		SpawnParams
	);
}


