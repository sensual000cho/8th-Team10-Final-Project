// CPEnemyDetectionComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPEnemyDetectionComponent.generated.h"

class ACPEnemyAIController;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPEnemyDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCPEnemyDetectionComponent();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void UpdateDetection();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DetectionTime = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NearDetectionRadius = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontDetectionRadius = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontDetectionHalfAngle = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UndetectedTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LoseTargetRadius = 1000.f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	bool bCanSeePlayer;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TObjectPtr<AActor> TargetActor = nullptr;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FVector LastKnownTargetLocation = FVector::ZeroVector;
	
private:
	FTimerHandle DetectionTimerHandle;
	
	UPROPERTY()
	TObjectPtr<ACPEnemyAIController> CachedAIController;
	UPROPERTY()
	TObjectPtr<APawn> CachedPlayerPawn;
};
