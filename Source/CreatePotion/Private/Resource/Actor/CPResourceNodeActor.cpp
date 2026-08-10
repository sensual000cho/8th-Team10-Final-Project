#include "Resource/Actor/CPResourceNodeActor.h"
#include "Data/CPResourceDefinition.h"
#include "Resource/System/CPResourceStateSubsystem.h"
#include "Items/CPDroppedItemBase.h"
#include "Resource/System/CPObjectPoolSubsystem.h"

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
	
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;
	
	UCPResourceStateSubsystem* StateSubsystem = GameInstance->GetSubsystem<UCPResourceStateSubsystem>();
	if (!StateSubsystem) return;
	
	UCPObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UCPObjectPoolSubsystem>();
	if (!Pool) return;
	
	AActor* AcquiredActor = Pool->AcquireActor(DroppedItemClass, {FRotator::ZeroRotator, GetActorLocation()});
	
	ACPDroppedItemBase* DroppedItem = Cast<ACPDroppedItemBase>(AcquiredActor);
	if (!DroppedItem)
	{
		Pool->ReleaseActor(AcquiredActor);
		return;
	}
	
	DroppedItem->Initialize(ResourceDefinition->HarvestedItem, ResourceDefinition->HarvestAmount,
		ResourceDefinition->Mesh.LoadSynchronous());
	
	const float RandomYaw = FMath::FRandRange(0.f, 360.f);
	const FVector DropDirection = FRotator(0.f, RandomYaw, 0.f).Vector();
	DroppedItem->StartDropMotion(DropDirection);
	
	StateSubsystem->MarkHarvested(NodeKey);
	
	Pool->ReleaseActor(this);
}

void ACPResourceNodeActor::Inspect(AActor* Interactor)
{
	//TODO: 도감 상태 갱신
}
