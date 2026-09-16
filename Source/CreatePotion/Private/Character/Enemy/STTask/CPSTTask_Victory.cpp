// CPSTTask_Victory.cpp


#include "Character/Enemy/STTask/CPSTTask_Victory.h"

#include "Character/Enemy/CPEnemyCharacter.h"
#include "StateTreeExecutionContext.h"

bool FCPSTCondition_Victory::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	
	return IsValid(Data.Enemy) && Data.Enemy->IsVictoryRequested();
}

EStateTreeRunStatus FCPSTTask_Victory::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	
	if (!IsValid(Data.Enemy))
	{
		return EStateTreeRunStatus::Failed;
	}
	
	return Data.Enemy->StartVictoryRequest() ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FCPSTTask_Victory::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	
	if (!IsValid(Data.Enemy))
	{
		return EStateTreeRunStatus::Failed;
	}
	
	if (Data.Enemy->IsVictoryPlaying())
	{
		return EStateTreeRunStatus::Running;
	}
	
	return Data.Enemy->DidVictoryCompletedNormally() ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}

void FCPSTTask_Victory::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	
	if (!IsValid(Data.Enemy))
	{
		return;
	}
	
	Data.Enemy->ClearVictoryRequest();
}
