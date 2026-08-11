#include "Lab/Actor/CPLabBell.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameMode/CPLabGameMode.h"
#include "GameState/CPLabGameState.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

#include "Kismet/GameplayStatics.h"
#include "NPC/CPNPCSpawner.h"

ACPLabBell::ACPLabBell()
{
	PrimaryActorTick.bCanEverTick = false;

	BellMesh =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BellMesh"));
	SetRootComponent(BellMesh);

	// 상호작용 Trace에 필요한 Visibility와 캐릭터 충돌만 사용
	BellMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BellMesh->SetCollisionObjectType(ECC_WorldDynamic);
	BellMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	BellMesh->SetCollisionResponseToChannel(
		ECC_Visibility,
		ECR_Block);
	BellMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void ACPLabBell::OnInteract_Implementation(AActor* Interactor)
{
	TryRingBell();
}

FText ACPLabBell::GetInteractionPrompt_Implementation()
{
	return FText::FromString(TEXT("F: 벨 울리기"));
}

bool ACPLabBell::CanInteract_Implementation(AActor* Interactor)
{
	const UWorld* World = GetWorld();
	const ACPLabGameState* LabState =
		World ? World->GetGameState<ACPLabGameState>() : nullptr;
	const UCPLabPotionSessionComponent* Session =
		LabState ? LabState->GetPotionSession() : nullptr;

	return Session &&
		Session->GetSessionState().Phase ==
			ECPLabPotionSessionPhase::WaitingForBell;
}

bool ACPLabBell::TryRingBell()
{
	UWorld* World = GetWorld();
	ACPLabGameMode* LabMode =
		World
			? World->GetAuthGameMode<ACPLabGameMode>()
			: nullptr;
	bool bSessionStarted = LabMode && LabMode->TryStartLabSession();

	// NPC 스폰
	if (bSessionStarted)
	{
		ACPNPCSpawner* Spawner = Cast<ACPNPCSpawner>(UGameplayStatics::GetActorOfClass(World, ACPNPCSpawner::StaticClass()));
		if (Spawner)
		{
			Spawner->StartSpawningSession();
		}
	}

	return bSessionStarted;
}
