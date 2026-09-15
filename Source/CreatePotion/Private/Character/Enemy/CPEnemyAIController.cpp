// CPEnemyAIController.cpp


#include "Character/Enemy/CPEnemyAIController.h"

#include "Character/Enemy/CPEnemyDetectionComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "GameFramework/Pawn.h"
#include "StateTree.h"

ACPEnemyAIController::ACPEnemyAIController()
{
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComponent"));
	DetectionComponent = CreateDefaultSubobject<UCPEnemyDetectionComponent>(TEXT("DetectionComponent"));
	
	// AIController가 사용하는 Brain으로 등록
	BrainComponent = StateTreeComponent;
	// 초기 데이터를 준비한 뒤 직접 시작
	bStartAILogicOnPossess = false;
	StateTreeComponent->SetStartLogicAutomatically(false);
}

void ACPEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	TryStartStateTree();
}

void ACPEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!IsValid(InPawn))
	{
		return;
	}

	// Possess한 시점의 적 위치가 활동의 시작점
	HomeLocation = InPawn->GetActorLocation();

	// UE_LOG(LogTemp, Log, TEXT("[EnemyAI] Possess 완료: %s / Home: %s"), *GetNameSafe(InPawn), *HomeLocation.ToString());

	TryStartStateTree();
}

void ACPEnemyAIController::OnUnPossess()
{
	// Pawn을 잃기 전에 실행 중인 행동과 이동 정리
	if (IsValid(StateTreeComponent))
	{
		StateTreeComponent->StopLogic(TEXT("Enemy unpossessed"));
	}

	StopMovement();

	Super::OnUnPossess();
}

void ACPEnemyAIController::TryStartStateTree()
{
	// 배치/스폰 방식에 따른 초기화 순서 차이 대응
	if (!HasActorBegunPlay() || !IsValid(GetPawn()))
	{
		return;
	}

	if (!IsValid(StateTreeComponent) || StateTreeComponent->IsRunning())
	{
		return;
	}

	if (!IsValid(EnemyStateTree))
	{
		return;
	}

	StateTreeComponent->SetStateTree(EnemyStateTree);
	StateTreeComponent->StartLogic();

	// UE_LOG(LogTemp, Log, TEXT("[EnemyAI] StateTree 실행 상태: %s"), StateTreeComponent->IsRunning() ? TEXT("Running") : TEXT("Stopped"));
}
