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

void ACPQuestBoard::HandleInteract(AActor* Interactor)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	if (!QuestManager) return;

	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();

	UCPInventoryComponent* Inventory = Interactor ? Interactor->FindComponentByClass<UCPInventoryComponent>() : nullptr;
	if (Inventory)
	{
		TMap<FName, int32> HeldItems = Inventory->GetItemCountsByID();

		for (const FName& QuestID : QuestManager->GetTrackedRandomQuestIDs())
		{
			if (QuestManager->GetQuestState(QuestID) != EQuestState::Accepted) continue;
			if (QuestManager->GetQuestCompletionType(QuestID) != EQuestCompletionType::ItemCollection) continue;
			if (!QuestManager->TryCompleteQuestByItem(QuestID, HeldItems)) continue;

			TArray<FName> RequiredIDs;
			int32 RequiredCount = 0;
			QuestManager->GetRequiredItemInfo(QuestID, RequiredIDs, RequiredCount);

			for (const FName& ItemID : RequiredIDs)
			{
				Inventory->RemoveItemByID(ItemID, RequiredCount);
			}

			const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
			UClass* ResultWidgetClass = UISettings ? UISettings->ResultWidgetClass.LoadSynchronous() : nullptr;
			if (UIManager && ResultWidgetClass)
			{
				UCPLabResultWidget* ResultWidget = Cast<UCPLabResultWidget>(UIManager->PushWidget(ResultWidgetClass));
				if (ResultWidget && !ResultWidget->InitializeResult(QuestID))
				{
					ResultWidget->RequestClose();
				}
			}
		}
	}

	if (QuestBoardWidgetClass && UIManager)
	{
		UIManager->PushWidget(QuestBoardWidgetClass);
	}
}