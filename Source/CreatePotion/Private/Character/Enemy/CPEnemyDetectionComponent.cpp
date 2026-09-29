// CPEnemyDetectionComponent.cpp


#include "Character/Enemy/CPEnemyDetectionComponent.h"

#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
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
        bFleeFromStench = false;
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
        bFleeFromStench = false;
        TargetActor = nullptr;
        return;
    }
	
	const UCPHealthComponent* PlayerHealth = CachedPlayerPawn->FindComponentByClass<UCPHealthComponent>();

	if (!IsValid(PlayerHealth) || PlayerHealth->IsDead())
	{
		bFleeFromStench = false;
		bCanSeePlayer = false;
		TargetActor = nullptr;
		return;
	}

    const FVector PlayerLocation = CachedPlayerPawn->GetActorLocation();
    const FVector EnemyLocation = EnemyPawn->GetActorLocation();

	// 냄새는 기존 시야·활동 구역 검사보다 먼저 판단한다.
	const UAbilitySystemComponent* PlayerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CachedPlayerPawn.Get());
	const float StenchRadius = bFleeFromStench ? FMath::Max(StenchDetectionRadius, StenchEscapeRadius) : StenchDetectionRadius;
	bFleeFromStench = PlayerASC && PlayerASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Stench"))) && FVector::DistSquared(PlayerLocation, EnemyLocation) < FMath::Square(StenchRadius);
	if (bFleeFromStench)
	{
		TargetActor = nullptr;
		return;
	}
	
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
	

