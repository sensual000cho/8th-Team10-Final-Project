#include "NPC/CPShopNPC.h"
#include "NPC/Components/CPNPCComponent.h"
#include "NPC/Components/CPShopContainerComponent.h" 
#include "Character/CPPlayerController.h"         
#include "GameFramework/Pawn.h"

ACPShopNPC::ACPShopNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	NPCComponent = CreateDefaultSubobject<UCPNPCComponent>(TEXT("NPCComponent"));
	NPCComponent->SetupAttachment(RootComponent);

	ShopContainerComponent = CreateDefaultSubobject<UCPShopContainerComponent>(TEXT("ShopContainerComponent"));
}

void ACPShopNPC::BeginPlay()
{
	Super::BeginPlay();
}

void ACPShopNPC::OpenShopUI(AActor* Interactor)
{
	if (!IsValid(Interactor)) return;

	APawn* InteractorPawn = Cast<APawn>(Interactor);
	if (!InteractorPawn) return;

	ACPPlayerController* PC = Cast<ACPPlayerController>(InteractorPawn->GetController());
	if (!IsValid(PC)) { return; }

	if (!IsValid(ShopContainerComponent)) { return; }

	PC->ToggleExternalContainerUI(ShopContainerComponent);
}