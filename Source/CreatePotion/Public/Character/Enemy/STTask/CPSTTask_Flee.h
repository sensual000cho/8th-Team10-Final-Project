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

// 같은 조건을 도주 진입과 종료에 사용한다.
USTRUCT(meta = (DisplayName = "CP Should Flee Stench", Category = "Enemy"))
struct CREATEPOTION_API FCPSTCondition_Stench : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()
	using FInstanceDataType = FCPSTCondition_StenchInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

// 추적과 같은 Move To를 쓰고 목적지만 Player 반대편으로 갱신한다.
USTRUCT(meta = (DisplayName = "CP Flee Stench", Category = "Enemy"))
struct CREATEPOTION_API FCPSTTask_Flee : public FStateTreeMoveToTask
{
	GENERATED_BODY()
	FCPSTTask_Flee();
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	bool UpdateDestination(FStateTreeExecutionContext& Context) const;
};
