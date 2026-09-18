#include "Resource/Actor/CPResourceNodeActor.h"

#include "Data/CPResourceDefinition.h"
#include "Resource/Component/CPHarvestComponent.h"

ACPResourceNodeActor::ACPResourceNodeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	
	HarvestComponent = CreateDefaultSubobject<UCPHarvestComponent>(TEXT("HarvestComponent"));
	HarvestComponent->SetupAttachment(Mesh);
}

void ACPResourceNodeActor::InitializeResource(const FCPResourceNodeKey& InNodeKey, UCPResourceDefinition* InDefinition, int32 InHarvestMultiplier)
{
	NodeKey = InNodeKey;
	ResourceDefinition = InDefinition;
	HarvestMultiplier = FMath::Max(1, InHarvestMultiplier);
	
	ApplyDefinition();
	
	if (ResourceDefinition)
	{
		HarvestComponent->Initialize(NodeKey, ResourceDefinition, HarvestMultiplier);
	}
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
	
	if (IsValid(HarvestComponent)){
		HarvestComponent->Initialize(FCPResourceNodeKey{}, nullptr, 1);
	}
	
	Mesh->SetStaticMesh(nullptr);
}

void ACPResourceNodeActor::ApplyDefinition()
{
	if (!ResourceDefinition) return;
	
	//TODO: 추후 비동기 로드 방식으로 전환 예정
	Mesh->SetStaticMesh(ResourceDefinition->Mesh.LoadSynchronous());
}