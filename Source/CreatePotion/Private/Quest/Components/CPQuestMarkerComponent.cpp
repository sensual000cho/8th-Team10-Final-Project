// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Components/CPQuestMarkerComponent.h"

#include "Quest/QuestManager.h"
#include "UI/Widgets/Common/Quest/CPQuestMarkerWidget.h"
#include "Components/CPInventoryComponent.h"
#include "Kismet/GameplayStatics.h"

UCPQuestMarkerComponent::UCPQuestMarkerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetWidgetSpace(EWidgetSpace::World);
	SetTwoSided(true);

	// 상호작용 트레이스/포커스 위치 계산에 끼어들지 않도록
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UCPQuestMarkerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>())
		{
			QuestManager->OnQuestUpdated.AddDynamic(this, &UCPQuestMarkerComponent::HandleQuestUpdated);
		}
	}

	UpdateQuestMarker();
}

void UCPQuestMarkerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>())
		{
			QuestManager->OnQuestUpdated.RemoveDynamic(this, &UCPQuestMarkerComponent::HandleQuestUpdated);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UCPQuestMarkerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		const FVector CameraLocation = CameraManager->GetCameraLocation();
		SetWorldRotation((CameraLocation - GetComponentLocation()).Rotation());
	}
}

void UCPQuestMarkerComponent::SetTrackedQuest(FName QuestID)
{
	TrackedQuestID = QuestID;
	UpdateQuestMarker();
}

void UCPQuestMarkerComponent::HandleQuestUpdated(FName QuestID, EQuestState NewState)
{
	if (TrackedQuestID.IsNone() || QuestID != TrackedQuestID) return;
	UpdateQuestMarker();
}

void UCPQuestMarkerComponent::UpdateQuestMarker()
{
	// 추적할 퀘스트가 없으면(모든 단계 종료) 마커 숨김
	SetVisibility(!TrackedQuestID.IsNone());
	if (TrackedQuestID.IsNone()) return;

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UQuestManager* QuestManager = GI ? GI->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return;

	TMap<FName, int32> HeldItemCounts;
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (UCPInventoryComponent* Inventory = PlayerPawn->FindComponentByClass<UCPInventoryComponent>())
		{
			HeldItemCounts = Inventory->GetItemCountsByID();
		}
	}

	const TArray<FName> Tracked = { TrackedQuestID };
	const EQuestMarkerState State = QuestManager->GetMarkerStateForQuests(Tracked, HeldItemCounts);

	if (UCPQuestMarkerWidget* MarkerWidget = Cast<UCPQuestMarkerWidget>(GetUserWidgetObject()))
	{
		MarkerWidget->SetMarkerState(State);
	}
}
