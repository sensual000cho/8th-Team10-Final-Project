#include "Resource/Actor/CPResourceNodeActor.h"
#include "Data/CPResourceDefinition.h"
#include "Resource/System/CPResourceStateSubsystem.h"
#include "Resource/Component/CPHarvestComponent.h"
#include "Resource/System/CPObjectPoolSubsystem.h"

ACPResourceNodeActor::ACPResourceNodeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	
	HarvestComponent = CreateDefaultSubobject<UCPHarvestComponent>(TEXT("HarvestComponent"));
}

void ACPResourceNodeActor::InitializeResource(const FCPResourceNodeKey& InNodeKey, UCPResourceDefinition* InDefinition)
{
	NodeKey = InNodeKey;
	ResourceDefinition = InDefinition;
	
	ApplyDefinition();
	
	if (ResourceDefinition)
	{
		HarvestComponent->Initialize(
			ResourceDefinition->HarvestData.HarvestedItem,
			ResourceDefinition->HarvestData.HarvestAmount,
			ResourceDefinition->Mesh.LoadSynchronous(),
			DroppedItemClass
			);
	}
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

FName ACPResourceNodeActor::GetInteractionName_Implementation()
{
	return FName(*ResourceDefinition->HarvestData.HarvestedItem->DisplayName.ToString());
}

bool ACPResourceNodeActor::CanInteract_Implementation(AActor* Interactor)
{
	return Interactor != nullptr
		&& ResourceDefinition != nullptr
		&& ResourceDefinition->HarvestData.HarvestedItem != nullptr;
}

float ACPResourceNodeActor::GetInteractionDuration_Implementation(AActor* Interactor)
{
	if (!ResourceDefinition) return 0.f;
	
	//TODO: 처음 발견하는 거라면 고정값
	//return InspectDuration;
	
	//아니라면 채집물 채집시간
	return ResourceDefinition->HarvestData.HarvestDuration;
}

void ACPResourceNodeActor::OnAcquireFromPool_Implementation()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

void ACPResourceNodeActor::OnReleaseToPool_Implementation()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	
	NodeKey = FCPResourceNodeKey{};
	ResourceDefinition = nullptr;
	
	Mesh->SetStaticMesh(nullptr);
}

void ACPResourceNodeActor::ApplyDefinition()
{
	if (!ResourceDefinition) return;
	
	//TODO: 추후 비동기 로드 방식으로 전환 예정
	Mesh->SetStaticMesh(ResourceDefinition->Mesh.LoadSynchronous());
}

void ACPResourceNodeActor::Harvest(AActor* Interactor)
{
	if (!Interactor) return;
	if (!ResourceDefinition) return;
	if (!HarvestComponent) return;
	if (!HarvestComponent->TryHarvest(Interactor)) return;
	
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	UCPResourceStateSubsystem* StateSubsystem = GI->GetSubsystem<UCPResourceStateSubsystem>();
	if (!StateSubsystem) return;
	
	StateSubsystem->MarkHarvested(NodeKey, ResourceDefinition->RespawnDuration);

	UWorld* World = GetWorld();
	if (!World) return;
	
	UCPObjectPoolSubsystem* Pool = World->GetSubsystem<UCPObjectPoolSubsystem>();
	if (!Pool) return;
	
	Pool->ReleaseActor(this);
}

void ACPResourceNodeActor::Inspect(AActor* Interactor)
{
	//TODO: 도감 상태 갱신
}
