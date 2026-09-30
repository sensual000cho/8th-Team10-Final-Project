#include "Character/Enemy/STTask/CPSTTask_Flee.h"

#include "Character/Enemy/CPEnemyAIController.h"
#include "Character/Enemy/CPEnemyDetectionComponent.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

bool FCPSTCondition_Stench::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const UCPEnemyDetectionComponent* Detection = IsValid(Data.AIController) ? Data.AIController->FindComponentByClass<UCPEnemyDetectionComponent>() : nullptr;
	return Detection && (Detection->bFleeFromStench != bInvert);
}

FCPSTTask_Flee::FCPSTTask_Flee()
{
	bSavedShouldCallTick = true;
}

bool FCPSTTask_Flee::UpdateDestination(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	const APawn* Enemy = IsValid(Data.AIController) ? Data.AIController->GetPawn() : nullptr;
	const UCPEnemyDetectionComponent* Detection = IsValid(Data.AIController) ? Data.AIController->FindComponentByClass<UCPEnemyDetectionComponent>() : nullptr;
	const APawn* Player = Detection ? Detection->GetStenchSource() : nullptr;
	if (!IsValid(Enemy) || !IsValid(Player)) return false;
	FVector Away = (Enemy->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal2D();
	if (Away.IsNearlyZero()) Away = Enemy->GetActorForwardVector().GetSafeNormal2D();
	// 도착 허용 반경 때문에 종료 거리 안에서 멈추지 않도록 여유를 둔다.
	Data.Destination = Player->GetActorLocation() + Away * (FMath::Max(Detection->StenchDetectionRadius, Detection->StenchEscapeRadius) + Data.AcceptableRadius + 50.0f);
	Data.Destination.Z = Enemy->GetActorLocation().Z;
	Data.TargetActor = nullptr;
	return true;
}

EStateTreeRunStatus FCPSTTask_Flee::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (!UpdateDestination(Context)) return EStateTreeRunStatus::Failed;
	return FStateTreeMoveToTask::EnterState(Context, Transition);
}

EStateTreeRunStatus FCPSTTask_Flee::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	if (!UpdateDestination(Context)) return EStateTreeRunStatus::Succeeded;
	return FStateTreeMoveToTask::Tick(Context, DeltaTime);
}
