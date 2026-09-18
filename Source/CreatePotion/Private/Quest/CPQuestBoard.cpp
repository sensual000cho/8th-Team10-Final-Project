// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/CPQuestBoard.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Quest/QuestManager.h"
#include "Components/CPInventoryComponent.h"
#include "Components/CPInteractableComponent.h"
#include "Settings/CPUISettings.h"
#include "UI/Widgets/Lab/CPLabResultWidget.h"

ACPQuestBoard::ACPQuestBoard()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractableComponent = CreateDefaultSubobject<UCPInteractableComponent>(TEXT("InteractableComponent"));
}

void ACPQuestBoard::BeginPlay()
{
	Super::BeginPlay();

	if (InteractableComponent)
	{
		InteractableComponent->SetInteractability(true);
		InteractableComponent->SetInteractionPrompt(FText::FromString(TEXT("F : 의뢰 확인하기")));
		InteractableComponent->SetTargetName(FName("QuestBoard"));
		InteractableComponent->OnActorInteractionCompleted.AddUniqueDynamic(this, &ACPQuestBoard::HandleInteract);
	}
}

void ACPQuestBoard::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InteractableComponent)
	{
		InteractableComponent->OnActorInteractionCompleted.RemoveDynamic(this, &ACPQuestBoard::HandleInteract);
	}

	Super::EndPlay(EndPlayReason);
}

void ACPQuestBoard::HandleInteract(AActor* Interactor)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	if (!QuestManager) return;

	UCPInventoryComponent* Inventory = Interactor ? Interactor->FindComponentByClass<UCPInventoryComponent>() : nullptr;
	if (Inventory)
	{
		TMap<FName, int32> HeldItems = Inventory->GetItemCountsByID();

		for (const FName& QuestID : QuestManager->GetTrackedRandomQuestIDs())
		{
			if (QuestManager->GetQuestState(QuestID) != EQuestState::Accepted) continue;
			if (QuestManager->GetQuestCompletionType(QuestID) != EQuestCompletionType::ItemCollection) continue;

			if (QuestManager->TryCompleteQuestByItem(QuestID, HeldItems))
			{
				TArray<FName> RequiredIDs;
				int32 RequiredCount = 0;
				QuestManager->GetRequiredItemInfo(QuestID, RequiredIDs, RequiredCount);

				for (const FName& ItemID : RequiredIDs)
				{
					Inventory->RemoveItemByID(ItemID, RequiredCount);
				}

				QuestManager->CreateResult(QuestID);
				
				const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
				UClass* ResultWidgetClass = UISettings ? UISettings->ResultWidgetClass.LoadSynchronous() : nullptr;
				if (UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>())
				{
					if (ResultWidgetClass)
					{
						UCPLabResultWidget* ResultWidget = Cast<UCPLabResultWidget>(UIManager->PushWidget(ResultWidgetClass));
						if (ResultWidget && !ResultWidget->InitializeResult(QuestID))
						{
							ResultWidget->RequestClose();
						}
					}
				}

				UE_LOG(LogTemp, Log, TEXT("[게시판] 랜덤 퀘스트 [%s] 완료 처리됨"), *QuestID.ToString());
			}
		}
	}

	if (!QuestBoardWidgetClass) return;

	if (UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>())
	{
		UIManager->PushWidget(QuestBoardWidgetClass);
	}
}