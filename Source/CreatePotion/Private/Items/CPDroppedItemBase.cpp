#include "Items/CPDroppedItemBase.h"
#include "Components/CPInventoryComponent.h"

ACPDroppedItemBase::ACPDroppedItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

void ACPDroppedItemBase::OnInteract_Implementation(AActor* Interactor)
{
	UCPInventoryComponent* Inventory = Interactor->FindComponentByClass<UCPInventoryComponent>();
	if (!Inventory) return;
	
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("OnInteract"));
	
	Inventory->TryGetItem(ItemData, Amount);
	
	Destroy();
}

FText ACPDroppedItemBase::GetInteractionPrompt_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("GetInteract"));
	
	return FText::FromString(TEXT("줍기"));
}

bool ACPDroppedItemBase::CanInteract_Implementation(AActor* Interactor)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("CanInteract"));
	
	return Interactor != nullptr && ItemData != nullptr && Amount > 0;
}

void ACPDroppedItemBase::Initialize(UCPForageableItemData* InItemData, int32 InAmount, UStaticMesh* InMesh)
{
	ItemData = InItemData;
	Amount = InAmount;
	Mesh->SetStaticMesh(InMesh);
}
