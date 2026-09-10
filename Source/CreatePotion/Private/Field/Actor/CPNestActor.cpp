#include "Field/Actor/CPNestActor.h"
#include "Data/CPResourceDefinition.h"
#include "Items/CPDroppedItemBase.h"
#include "Resource/System/CPObjectPoolSubsystem.h"

ACPNestActor::ACPNestActor()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	DropPoint = CreateDefaultSubobject<USceneComponent>(TEXT("DropPoint"));
	DropPoint->SetupAttachment(Root);
}

void ACPNestActor::PotionReaction()
{
	Super::PotionReaction();
	
	if (!DropItemData) return;
	if (DropItemData->HarvestData.HarvestAmount <= 0) return;
	if (!DropItemData->Mesh) return;
	if (!DropItemData->HarvestData.HarvestedItem) return;
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	UCPObjectPoolSubsystem* Pool = World->GetSubsystem<UCPObjectPoolSubsystem>();
	if (!Pool) return;
	
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
}

bool ACPNestActor::CanReactToTag(const FGameplayTag& EventTag) const
{
	return EventTag.MatchesTagExact(ReactionEventTag);
}
