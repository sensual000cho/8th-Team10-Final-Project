#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "Tasks/StateTreeMoveToTask.h"
#include "CPSTTask_Flee.generated.h"

class ACPEnemyAIController;

USTRUCT()
struct CREATEPOTION_API FCPSTCondition_StenchInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ACPEnemyAIController> AIController = nullptr;
};

// 악취 도주 상태를 확인하는 전환 조건
USTRUCT(meta = (DisplayName = "CP Should Flee Stench", Category = "Enemy"))
struct CREATEPOTION_API FCPSTCondition_Stench : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()
	using FInstanceDataType = FCPSTCondition_StenchInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	// 도주 종료 조건에서는 결과 반전
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

// Player로부터 멀어지는 목적지를 갱신하는 Move To Task
USTRUCT(meta = (DisplayName = "CP Flee Stench", Category = "Enemy"))
struct CREATEPOTION_API FCPSTTask_Flee : public FStateTreeMoveToTask
{
	GENERATED_BODY()
	FCPSTTask_Flee();
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	// 도주 종료 거리 밖의 목적지 계산
	bool UpdateDestination(FStateTreeExecutionContext& Context) const;
};
