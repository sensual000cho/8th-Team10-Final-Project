// CPEnemyDetectionComponent.cpp


#include "Character/Enemy/CPEnemyDetectionComponent.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Enemy/CPEnemyAIController.h"
#include "Character/CPHealthComponent.h"

UCPEnemyDetectionComponent::UCPEnemyDetectionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}


void UCPEnemyDetectionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CachedAIController = Cast<ACPEnemyAIController>(GetOwner());

	GetWorld()->GetTimerManager().SetTimer(
		DetectionTimerHandle,
		this,
		&UCPEnemyDetectionComponent::UpdateDetection,
		DetectionTime,
		true
		);
	
}

void UCPEnemyDetectionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(DetectionTimerHandle);
	
	Super::EndPlay(EndPlayReason);
}


void UCPEnemyDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

void UCPEnemyDetectionComponent::UpdateDetection()
{
    bCanSeePlayer = false;
    
	const ACPEnemyAIController* EnemyController = CachedAIController.Get();

    UWorld* World = GetWorld();

    if (!IsValid(EnemyController) || !IsValid(World))
    {
        TargetActor = nullptr;
        return;
    }

    APawn* EnemyPawn = EnemyController->GetPawn();

    if (!IsValid(CachedPlayerPawn))
    {
        CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    }

    if (!IsValid(EnemyPawn) || !IsValid(CachedPlayerPawn))
    {
        TargetActor = nullptr;
        return;
    }
	
	const UCPHealthComponent* PlayerHealth = CachedPlayerPawn->FindComponentByClass<UCPHealthComponent>();

	if (!IsValid(PlayerHealth) || PlayerHealth->IsDead())
	{
		bCanSeePlayer = false;
		TargetActor = nullptr;
		return;
	}

    const FVector PlayerLocation = CachedPlayerPawn->GetActorLocation();
    const FVector EnemyLocation = EnemyPawn->GetActorLocation();
	
    const FVector HomeToPlayer = PlayerLocation - EnemyController->GetHomeLocation();
    const float HomeDistanceSquared = HomeToPlayer.SizeSquared2D();
	
    if (HomeDistanceSquared > FMath::Square(LoseTargetRadius))
    {
   //  	UE_LOG(
			// LogTemp,
			// Log,
			// TEXT("[EnemyDetection] %s: 활동 범위 이탈로 추적 해제"),
			// *GetNameSafe(EnemyPawn));
    	
        TargetActor = nullptr;
        return;
    }

    const bool bHasTarget = IsValid(TargetActor);

	// 최초 발견시에만
    if (!bHasTarget)
    {
        const FVector ToPlayer = PlayerLocation - EnemyLocation;
        const float DistanceSquared = ToPlayer.SizeSquared2D();
        const FVector DirectionToPlayer = ToPlayer.GetSafeNormal2D();
        const FVector EnemyForward = EnemyPawn->GetActorForwardVector().GetSafeNormal2D();

        const float Dot = FVector::DotProduct(EnemyForward, DirectionToPlayer);
        const float CosThreshold = FMath::Cos(FMath::DegreesToRadians(FrontDetectionHalfAngle));
        const bool bInFrontCone = Dot >= CosThreshold;

        const bool bInDetectionRange = DistanceSquared <= FMath::Square(NearDetectionRadius) || (DistanceSquared <= FMath::Square(FrontDetectionRadius) && bInFrontCone);

        if (!bInDetectionRange)
        {
            return;
        }
    }
	
    FHitResult Hit;

    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(EnemyPawn);
    CollisionParams.bTraceComplex = false;

    const bool bHit = World->LineTraceSingleByChannel(
        Hit,
        EnemyLocation,
        PlayerLocation,
        ECC_Visibility,
        CollisionParams
    );

    bCanSeePlayer = !bHit || Hit.GetActor() == CachedPlayerPawn.Get();

    if (bCanSeePlayer)
    {
    	if (!IsValid(TargetActor))
    	{
    // 		UE_LOG(
				// LogTemp,
				// Log,
				// TEXT("[EnemyDetection] %s: 플레이어 발견"),
				// *GetNameSafe(EnemyPawn));
    	}
    	
        TargetActor = CachedPlayerPawn;
        LastKnownTargetLocation = PlayerLocation;
    }
	
}
	

