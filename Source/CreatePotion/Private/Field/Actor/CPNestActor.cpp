#include "Field/Actor/CPNestActor.h"
#include "Data/CPResourceDefinition.h"
#include "Items/CPDroppedItemBase.h"
#include "Resource/System/CPObjectPoolSubsystem.h"

ACPNestActor::ACPNestActor()
{
	DropPoint = CreateDefaultSubobject<USceneComponent>(TEXT("DropPoint"));
	DropPoint->SetupAttachment(Root);
}

bool ACPNestActor::PotionReaction(const FGameplayTag& EventTag, AActor* InstigatorActor)
{
	Super::PotionReaction(EventTag, InstigatorActor);
	
	if (!DropItemData) return false;

	if (DropItemData->HarvestData.HarvestAmount <= 0) return false;

	if (!DropItemData->Mesh) return false;

	if (!DropItemData->HarvestData.HarvestedItem) return false;
	
	UWorld* World = GetWorld();
	if (!World) return false;
	
	UCPObjectPoolSubsystem* Pool = World->GetSubsystem<UCPObjectPoolSubsystem>();
	if (!Pool) return false;
	
	AActor* AcquiredActor = Pool->AcquireActor(DroppedItemClass, {FRotator::ZeroRotator,
		DropPoint->GetComponentLocation()});
	
	ACPDroppedItemBase* DroppedItem = Cast<ACPDroppedItemBase>(AcquiredActor);
	if (!DroppedItem)
	{
		if (AcquiredActor)
		{
			Pool->ReleaseActor(AcquiredActor);
		}
	}
	
	DroppedItem->Initialize(DropItemData->HarvestData.HarvestedItem, DropItemData->HarvestData.HarvestAmount, DropItemData->Mesh.LoadSynchronous());
	
	const float RandomYaw = FMath::FRandRange(0.f, 360.f);
	const FVector DropDirection =  FRotator(0.f, RandomYaw, 0.f).Vector();
	
	DroppedItem->StartDropMotion(DropDirection);
	
	Destroy();
	
	return true;
}