#include "Resource/Actor/CPResourceNodeActor.h"
#include "Data/CPResourceDefinition.h"
#include "Resource/System/CPResourceStateSubsystem.h"
#include "Items/CPDroppedItemBase.h"

ACPResourceNodeActor::ACPResourceNodeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

void ACPResourceNodeActor::InitializeResource(const FCPResourceNodeKey& InNodeKey, UCPResourceDefinition* InDefinition)
{
	NodeKey = InNodeKey;
	ResourceDefinition = InDefinition;
	
	ApplyDefinition();
}

void ACPResourceNodeActor::OnInteract_Implementation(AActor* Interactor)
{
	//TODO: 처음 발견하는 거라면 조사하기
	//Inspect(Interactor);
	
	//지금은 일단 채집 바로
	Harvest(Interactor);
}

FText ACPResourceNodeActor::GetInteractionPrompt_Implementation()
{
	//TODO: 처음 발견하는 거라면 조사하기
	//return FText::FromString(TEXT("조사하기"));
	
	//지금은 일단 채집 바로
	return FText::FromString(TEXT("채집하기"));
}

bool ACPResourceNodeActor::CanInteract_Implementation(AActor* Interactor)
{
	return Interactor != nullptr
		&& ResourceDefinition != nullptr
		&& ResourceDefinition->HarvestedItem != nullptr;
}

float ACPResourceNodeActor::GetInteractionDuration_Implementation(AActor* Interactor)
{
	if (!ResourceDefinition) return 0.f;
	
	//TODO: 처음 발견하는 거라면 고정값
	//return InspectDuration;
	
	//아니라면 채집물 채집시간
	return ResourceDefinition->HarvestDuration;
}

void ACPResourceNodeActor::ApplyDefinition()
{
	if (!ResourceDefinition) return;
	
	Mesh->SetStaticMesh(ResourceDefinition->Mesh.LoadSynchronous());
}

void ACPResourceNodeActor::Harvest(AActor* Interactor)
{
	if (!Interactor) return;
	if (!ResourceDefinition) return;
	
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;
	
	UCPResourceStateSubsystem* StateSubsystem = GameInstance->GetSubsystem<UCPResourceStateSubsystem>();
	if (!StateSubsystem) return;
	
	ACPDroppedItemBase* DroppedItem = GetWorld()->SpawnActor<ACPDroppedItemBase>(ACPDroppedItemBase::StaticClass(), GetActorTransform());
	if (!DroppedItem) return;
	
	DroppedItem->Initialize(ResourceDefinition->HarvestedItem, ResourceDefinition->HarvestAmount, ResourceDefinition->Mesh.LoadSynchronous());
	
	StateSubsystem->MarkHarvested(NodeKey);
	
	Destroy();
}

void ACPResourceNodeActor::Inspect(AActor* Interactor)
{
	//TODO: 도감 상태 갱신
}
