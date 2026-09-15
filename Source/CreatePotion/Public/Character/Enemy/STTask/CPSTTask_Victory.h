// CPSTTask_Victory.h

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "StateTreeTaskBase.h"
#include "CPSTTask_Victory.generated.h"

class ACPEnemyCharacter;

USTRUCT()
struct CREATEPOTION_API FCPSTCondition_VictoryInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ACPEnemyCharacter> Enemy = nullptr;
};

USTRUCT(meta = (DisplayName = "CP Victory", Category = "Enemy"))
struct CREATEPOTION_API FCPSTCondition_Victory : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCPSTCondition_VictoryInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}
	
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

USTRUCT()
struct CREATEPOTION_API FCPSTTask_VictoryInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ACPEnemyCharacter> Enemy = nullptr;
};

USTRUCT(meta = (DisplayName = "CP Victory", Category = "Enemy"))
struct CREATEPOTION_API FCPSTTask_Victory : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCPSTTask_VictoryInstanceData;

	FCPSTTask_Victory()
	{
		bShouldCallTick = true;
	}

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override; 
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};