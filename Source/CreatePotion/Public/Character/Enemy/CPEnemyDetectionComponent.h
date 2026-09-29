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
	// 도주 중인 경우 악취의 원인인 Player 반환
	APawn* GetStenchSource() const { return bFleeFromStench ? CachedPlayerPawn.Get() : nullptr; }

	// 시야와 관계없이 악취 도주를 시작하는 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stench", meta = (ClampMin = "0.0", Units = "cm"))
	float StenchDetectionRadius = 500.0f;
	// 도주 종료 거리. 감지 거리 이상으로 적용해 경계에서 반복 전환 방지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stench", meta = (ClampMin = "0.0", Units = "cm"))
	float StenchEscapeRadius = 700.0f;
	// StateTree에서 참조할 악취 도주 여부
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
