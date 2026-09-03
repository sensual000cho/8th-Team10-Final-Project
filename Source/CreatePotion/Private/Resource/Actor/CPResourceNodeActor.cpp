#include "Resource/Actor/CPResourceNodeActor.h"
#include "Data/CPResourceDefinition.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
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
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
		if (CodexSubsystem)
		{
			// 처음 발견하는 거라면 조사하기
			if (!CodexSubsystem->GetForageableEntry(ResourceDefinition->HarvestData.HarvestedItem))
			{
				Inspect(Interactor);
			}

			// 아니라면 수확
			else
			{
				Harvest(Interactor);
			}
		}
	}
}

FText ACPResourceNodeActor::GetInteractionPrompt_Implementation()
{
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
		if (CodexSubsystem)
		{
			//처음 발견하는 거라면 조사하기
			if (!CodexSubsystem->GetForageableEntry(ResourceDefinition->HarvestData.HarvestedItem))
			{
				return FText::FromString(TEXT("조사하기"));
			}

			//아니라면 채집
			else
			{
				return FText::FromString(TEXT("채집하기"));
			}
		}
	}
	
	return FText::FromString(TEXT("None"));
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
	
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
		if (CodexSubsystem)
		{
			// 처음 발견하는 거라면 조사시간
			if (!CodexSubsystem->GetForageableEntry(ResourceDefinition->HarvestData.HarvestedItem))
			{
				return InspectDuration;
			}

			// 아니라면 채집물 채집시간
			else
			{
				return ResourceDefinition->HarvestData.HarvestDuration;
			}
		}
	}
	
	return 0.f;
}

void ACPResourceNodeActor::OnInteractionStarted_Implementation(AActor* Interactor)
{
	if (!IsValid(Interactor) || !ResourceDefinition)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UCPCodexSubsystem* CodexSubsystem = GameInstance ? GameInstance->GetSubsystem<UCPCodexSubsystem>() : nullptr;

	if (!CodexSubsystem)
	{
		return;
	}
	
	USkeletalMeshComponent* CharacterMesh = Interactor->FindComponentByClass<USkeletalMeshComponent>();
	UAnimInstance* AnimInstance = IsValid(CharacterMesh) ? CharacterMesh->GetAnimInstance() : nullptr;

	// 조사일 경우 조사 몽타주 재생
	if (!CodexSubsystem->GetForageableEntry(ResourceDefinition->HarvestData.HarvestedItem))
	{
		if (IsValid(AnimInstance) && IsValid(InspectMontage))
		{
			AnimInstance->Montage_Play(InspectMontage);
		}
		return;
	}
	
	// 조사가 아닐 경우 채집 몽타주 재생
	if (IsValid(AnimInstance) && IsValid(HarvestMontage))
	{
		AnimInstance->Montage_Play(HarvestMontage);
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
	
	UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
	if (!CodexSubsystem) return;
	
	CodexSubsystem->RecordForageableEntry(ResourceDefinition->HarvestData.HarvestedItem);

	UWorld* World = GetWorld();
	if (!World) return;
	
	UCPObjectPoolSubsystem* Pool = World->GetSubsystem<UCPObjectPoolSubsystem>();
	if (!Pool) return;
	
	Pool->ReleaseActor(this);
}

void ACPResourceNodeActor::Inspect(AActor* Interactor)
{
	if (!Interactor) return;
	if (!ResourceDefinition) return;
	
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
	if (!CodexSubsystem) return;
	
	CodexSubsystem->RecordForageableEntry(ResourceDefinition->HarvestData.HarvestedItem);
}
