#include "Lab/Actor/CPLabBell.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameState/CPLabGameState.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

#include "UI/HUD/CPLabHUD.h"

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
	if (!TryRingBell()) return;
	if (APawn* InteractorPawn = Cast<APawn>(Interactor))
	{
		if (APlayerController* PC = Cast<APlayerController>(InteractorPawn->GetController()))
		{
			if (ACPLabHUD* LabHUD = Cast<ACPLabHUD>(PC->GetHUD()))
			{
				LabHUD->StartLabCraftingFlow();
			}
		}
	}
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

	return Session && !Session->HasActiveRequest();
}

bool ACPLabBell::TryRingBell()
{
	return CanInteract_Implementation(nullptr);
}