#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Data/NPC/CPNPCTargetData.h"
#include "GameInstance/Subsystem/CPNPCSubsystem.h"
#include "CPNPCMoveComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCPNPC, Log, All);

class AAIController;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPNPCMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPNPCMoveComponent();

	UFUNCTION(BlueprintCallable, Category = "AI Movement")
	void StartMoving();

	UFUNCTION(BlueprintCallable, Category = "AI Movement")
	void StopMoving();

	UFUNCTION(BlueprintCallable, Category = "AI Movement")
	void ApplySpeed(float Speed);

	UFUNCTION(BlueprintCallable, Category = "RVO")
	void SetRVOAvoidanceEnabled(bool bEnable);

	int32 GetCurrentPointIndex() const { return CurrentPointIndex; }
	void RestoreMovementState(const FCPNPCMovementSaveData& SaveData);

	bool IsMovePaused() const { return bIsMovePaused; }

	void OnDisplacementTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void RespawnToPreDisplaceState();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeMovementData();
	void MoveToCurrentIndex();

	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

private:
	void SaveMovementState();
	FString GetCurrentLevelIdentifier() const;
	FName GetMovementStorageKey() const;

	FVector FindValidNavLocation(const FVector& InLocation) const;

private:
	UPROPERTY()
	TObjectPtr<AAIController> AIController;

	UPROPERTY()
	bool bIsMoving = false;

	// 현재 활성화된 경로 데이터
	UPROPERTY(VisibleAnywhere, Category = "Route")
	TArray<FCPTargetPoint> TargetPoints;

	UPROPERTY(VisibleAnywhere, Category = "Route")
	int32 CurrentPointIndex = 0;

	UPROPERTY(VisibleAnywhere, Category = "Route")
	bool bLoop = true;

	UPROPERTY(EditAnywhere, Category = "AI Movement")
	float AcceptanceRadius = 10.0f;

	UPROPERTY(EditAnywhere, Category = "AI Movement")
	float MoveSpeed = 100.0f;

	UPROPERTY(EditAnywhere, Category = "RVO")
	float AvoidanceRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category = "RVO")
	float AvoidanceWeight = 0.5f;

	UPROPERTY()
	bool bIsMovePaused = false;

	FTimerHandle MoveWaitTimerHandle;
	FVector PreDisplaceLocation = FVector::ZeroVector;
	FRotator PreDisplaceRotation = FRotator::ZeroRotator;

	bool bWasMovingBeforeDisplace = false;
};