#include "Character/Enemy/STTask/CPSTTask_Attack.h"

#include "Character/Enemy/CPEnemyCharacter.h"
#include "StateTreeExecutionContext.h"

bool FCPSTCondition_CanAttack::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);

	return IsValid(Data.Enemy)&& Data.Enemy->CanAttackTarget(Data.TargetActor.Get());
}

EStateTreeRunStatus FCPSTTask_Attack::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	Data.bStartedAttack = false;

	if (!IsValid(Data.Enemy))
	{
		return EStateTreeRunStatus::Failed;
	}

	Data.bStartedAttack = Data.Enemy->TryStartAttack(Data.TargetActor.Get());

	return Data.bStartedAttack ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FCPSTTask_Attack::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);

	if (!Data.bStartedAttack || !IsValid(Data.Enemy))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (Data.Enemy->IsAttacking())
	{
		return EStateTreeRunStatus::Running;
	}

	return Data.Enemy->DidLastAttackCompleteNormally() ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}

void FCPSTTask_Attack::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	if (Data.bStartedAttack && IsValid(Data.Enemy))
	{
		Data.Enemy->CancelAttack();
	}

	Data.bStartedAttack = false;
}