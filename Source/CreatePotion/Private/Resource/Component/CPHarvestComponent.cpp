#include "Resource/Component/CPHarvestComponent.h"
#include "Components/CPInventoryComponent.h"
#include "Items/CPDroppedItemBase.h"
#include "Resource/System/CPObjectPoolSubsystem.h"

UCPHarvestComponent::UCPHarvestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPHarvestComponent::Initialize(UCPForageableItemData* InItemData, int32 InAmount)
{
	ItemData = InItemData;
	Amount = InAmount;
}

bool UCPHarvestComponent::TryHarvest(AActor* Interactor)
{
	if (!Interactor) return false;
	
	AActor* Owner = GetOwner();
	if (!Owner) return false;
	
	if (!ItemData) return false;
	if (Amount <= 0) return false;
	
	UWorld* World = GetWorld();
	if (!World) return false;
	
	UCPObjectPoolSubsystem* Pool = World->GetSubsystem<UCPObjectPoolSubsystem>();
	if (!Pool) return false;
	
	UCPInventoryComponent* Inventory = Interactor->FindComponentByClass<UCPInventoryComponent>();
	if (!Inventory) return false;
	
	Inventory->TryGetItemFromData(ItemData, Amount);
	
	return true;
}
