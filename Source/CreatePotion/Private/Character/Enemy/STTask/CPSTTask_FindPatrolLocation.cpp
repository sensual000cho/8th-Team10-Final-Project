// CPSTTask_FindPatrolLocation.cpp


#include "Character/Enemy/STTask/CPSTTask_FindPatrolLocation.h"

#include "Character/Enemy/CPEnemyAIController.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FCPSTTask_FindPatrolLocation::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ACPEnemyAIController* AIController = InstanceData.AIController.Get();

	if (!IsValid(AIController) || !IsValid(AIController->GetPawn()))
	{
		return EStateTreeRunStatus::Failed;
	}

	UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetCurrent(AIController->GetWorld());

	if (!IsValid(NavigationSystem))
	{
		return EStateTreeRunStatus::Failed;
	}

	FNavLocation ResultLocation;

	// 현재 위치가 아니라 최초 배치 위치를 중심으로 선정.
	// 본인의 구역에서 벗어나는 것을 방지.
	const bool bFoundLocation = NavigationSystem->GetRandomReachablePointInRadius(
			AIController->GetHomeLocation(),
			InstanceData.PatrolRadius,
			ResultLocation
			);

	if (!bFoundLocation)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.PatrolLocation = ResultLocation.Location;

	UE_LOG(LogTemp, Log, TEXT("[EnemyAI] 배회 목적지: %s"), *InstanceData.PatrolLocation.ToString());

	// 목적지가 유지되도록 부모 Task는 실행 상태로 유지.
	// 실제 이동 완료 여부는 Move To가 판단.
	return EStateTreeRunStatus::Running;
}
