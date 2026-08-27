#include "Resource/Component/CPHarvestComponent.h"
#include "Items/CPDroppedItemBase.h"
#include "Resource/System/CPObjectPoolSubsystem.h"

UCPHarvestComponent::UCPHarvestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPHarvestComponent::Initialize(UCPForageableItemData* InItemData, int32 InAmount, UStaticMesh* InDroppedItemMesh,
	TSubclassOf<ACPDroppedItemBase> InDroppedItemClass)
{
	ItemData = InItemData;
	Amount = InAmount;
	DroppedItemMesh = InDroppedItemMesh;
	DroppedItemClass = InDroppedItemClass;
}

bool UCPHarvestComponent::TryHarvest(AActor* Interactor)
{
	if (!Interactor) return false;
	
	AActor* Owner = GetOwner();
	if (!Owner) return false;
	
	if (!ItemData) return false;
	if (Amount <= 0) return false;
	if (!DroppedItemMesh) return false;
	if (!DroppedItemClass) return false;
	
	UWorld* World = GetWorld();
	if (!World) return false;
	
	UCPObjectPoolSubsystem* Pool = World->GetSubsystem<UCPObjectPoolSubsystem>();
	if (!Pool) return false;
	
	AActor* AcquiredActor = Pool->AcquireActor(DroppedItemClass, {FRotator::ZeroRotator, Owner->GetActorLocation()});
	
	ACPDroppedItemBase* DroppedItem = Cast<ACPDroppedItemBase>(AcquiredActor);
	if (!DroppedItem)
	{
		if (AcquiredActor)
		{
			Pool->ReleaseActor(AcquiredActor);
		}
		
		return false;
	}
	
	DroppedItem->Initialize(ItemData, Amount, DroppedItemMesh);
	
	const float RandomYaw = FMath::FRandRange(0.f, 360.f);
	const FVector DropDirection =  FRotator(0.f, RandomYaw, 0.f).Vector();
	
	DroppedItem->StartDropMotion(DropDirection);
	
	return true;
}
