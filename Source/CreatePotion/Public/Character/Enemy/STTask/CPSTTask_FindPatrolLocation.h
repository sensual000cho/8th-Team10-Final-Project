// CPSTTask_FindPatrolLocation.h

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "CPSTTask_FindPatrolLocation.generated.h"

class ACPEnemyAIController;

// 각 AI가 개별적으로 사용하는 Task 데이터
USTRUCT()
struct CREATEPOTION_API FCPSTTask_FindPatrolLocationInstanceData
{
	GENERATED_BODY()

	// StateTree의 AIController Context와 연결
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ACPEnemyAIController> AIController = nullptr;

	// Patrol 반경
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "1.0", Units = "cm"))
	float PatrolRadius = 500.f;

	// Move To Task에 전달할 목적지
	UPROPERTY(EditAnywhere, Category = "Output")
	FVector PatrolLocation = FVector::ZeroVector;
};

USTRUCT(meta = (DisplayName = "CP Find Patrol Location", Category = "Enemy"))
struct CREATEPOTION_API FCPSTTask_FindPatrolLocation : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCPSTTask_FindPatrolLocationInstanceData;

	FCPSTTask_FindPatrolLocation()
	{
		// 목적지는 상태 진입 시 한 번만 선정
		bShouldCallTick = false;
	}

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};