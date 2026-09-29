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
	// 도주 목적지를 계산할 때만 Player의 현재 위치를 참조한다.
	APawn* GetStenchSource() const { return bFleeFromStench ? CachedPlayerPawn.Get() : nullptr; }

	// 벽과 시야에 관계없이 냄새를 감지하며, 종료 거리를 더 크게 둔다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stench", meta = (ClampMin = "0.0", Units = "cm"))
	float StenchDetectionRadius = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stench", meta = (ClampMin = "0.0", Units = "cm"))
	float StenchEscapeRadius = 700.0f;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|Stench")
	bool bFleeFromStench = false;
	
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
