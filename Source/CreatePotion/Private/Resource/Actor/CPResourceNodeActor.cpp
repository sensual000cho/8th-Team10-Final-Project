#include "Resource/Actor/CPResourceNodeActor.h"

#include "Character/CPInteractionComponent.h"
#include "Components/CPInteractableComponent.h"
#include "Components/WidgetComponent.h"
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

void ACPResourceNodeActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsValid(InteractableComponent))
	{
		InteractableComponent->OnActorInteractionStarted.AddUniqueDynamic(this, &ACPResourceNodeActor::HandleInteractionStarted);
		InteractableComponent->OnActorInteractionCompleted.AddUniqueDynamic(this, &ACPResourceNodeActor::HandleInteract);
	}
	
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
	if (!CodexSubsystem) return;
	
	CodexSubsystem->OnForageableCodexUpdated.AddUniqueDynamic(this, &ACPResourceNodeActor::HandleResourceDiscovered);
}

void ACPResourceNodeActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(InteractableComponent))
	{
		InteractableComponent->OnActorInteractionStarted.RemoveDynamic(this, &ACPResourceNodeActor::HandleInteractionStarted);
		InteractableComponent->OnActorInteractionCompleted.RemoveDynamic(this, &ACPResourceNodeActor::HandleInteract);
	}
	
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
	if (!CodexSubsystem) return;
	
	CodexSubsystem->OnForageableCodexUpdated.RemoveDynamic(this, &ACPResourceNodeActor::HandleResourceDiscovered);
	
	Super::EndPlay(EndPlayReason);
	
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
	
	// 도감 상태에 따라 컴포넌트 설정값 동적 세팅
	if (InteractableComponent && ResourceDefinition)
	{
		const bool bValid = ResourceDefinition->HarvestData.HarvestedItem != nullptr;
		InteractableComponent->SetInteractability(bValid);
		
		
		UGameInstance* GI = GetGameInstance();
		UCPCodexSubsystem* CodexSubsystem = GI ? GI->GetSubsystem<UCPCodexSubsystem>() : nullptr;
		const bool bIsDiscovered = CodexSubsystem ? CodexSubsystem->GetForageableEntry(ResourceDefinition->HarvestData.HarvestedItem) : false;
		
		FText PromptText = bIsDiscovered ? FText::FromString(TEXT("채집하기")) : FText::FromString(TEXT("조사하기"));;
		FName ItemName = FName(*ResourceDefinition->HarvestData.HarvestedItem->DisplayName.ToString());
		
		UE_LOG(LogTemp, Warning, TEXT("[InitializeResource] Prompt=%s, ItemName=%s"), *PromptText.ToString(), *ItemName.ToString());
		
		InteractableComponent->SetInteractionPrompt(PromptText);
		InteractableComponent->SetTargetName(ItemName);
		
		UE_LOG(LogTemp, Warning, TEXT("[InitializeResource] Prompt=%s, ItemName=%s"), *PromptText.ToString(), *ItemName.ToString());
		
		float Duration = bIsDiscovered ? ResourceDefinition->HarvestData.HarvestDuration : InspectDuration;
		InteractableComponent->SetInteractionDuration(Duration);
		
		
	}
	
	
	
}

void ACPResourceNodeActor::HandleInteract(AActor* Interactor)
{
	// UE_LOG(LogTemp, Warning, TEXT("HandleInteract 함수 호출됨"));
	if (!ResourceDefinition) return;
	
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
		if (CodexSubsystem)
		{
			// 처음 발견하는 거라면 조사하기
			if (!CodexSubsystem->GetForageableEntry(ResourceDefinition->HarvestData.HarvestedItem))
			{
				// UE_LOG(LogTemp, Warning, TEXT("조사중..."));
				Inspect(Interactor);
			}

			// 아니라면 수확
			else
			{
				// UE_LOG(LogTemp, Warning, TEXT("채집중..."));
				Harvest(Interactor);
			}
		}
	}
}

void ACPResourceNodeActor::HandleInteractionStarted(AActor* Interactor)
{
	UE_LOG(LogTemp, Warning, TEXT("[ResourceActor] 채집 상호작용 호출됨"));
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
	InteractableComponent->SetInteractability(false);
	
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

void ACPResourceNodeActor::HandleResourceDiscovered()
{
	// 조사 완료된 재료 프롬프트 변경
	if (!ResourceDefinition || !InteractableComponent) return;
	UGameInstance* GI = GetGameInstance();
	UCPCodexSubsystem* CodexSubsystem = GI ? GI->GetSubsystem<UCPCodexSubsystem>() : nullptr;
	if (!CodexSubsystem) return;
	
	if (!CodexSubsystem->GetForageableEntry(ResourceDefinition->HarvestData.HarvestedItem)) return;
	
	InteractableComponent->SetInteractionPrompt(FText::FromString(TEXT("채집하기")));
}


