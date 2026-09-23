// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Quest/CPCommonGoalWidget.h"

#include "Components/TextBlock.h"
#include "Quest/QuestManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CPInventoryComponent.h"

void UCPCommonGoalWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>())
		{
			QuestManager->OnQuestUpdated.AddDynamic(this, &UCPCommonGoalWidget::HandleQuestUpdated);
		}
	}

	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (UCPInventoryComponent* Inventory = PlayerPawn->FindComponentByClass<UCPInventoryComponent>())
		{
			Inventory->OnContainerUpdated.AddDynamic(this, &UCPCommonGoalWidget::HandleInventoryUpdated);
		}
	}

	UpdateGoalDisplay();
}

void UCPCommonGoalWidget::NativeDestruct()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>())
		{
			QuestManager->OnQuestUpdated.RemoveDynamic(this, &UCPCommonGoalWidget::HandleQuestUpdated);
		}
	}

	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (UCPInventoryComponent* Inventory = PlayerPawn->FindComponentByClass<UCPInventoryComponent>())
		{
			Inventory->OnContainerUpdated.RemoveDynamic(this, &UCPCommonGoalWidget::HandleInventoryUpdated);
		}
	}

	Super::NativeDestruct();
}

void UCPCommonGoalWidget::HandleQuestUpdated(FName QuestID, EQuestState NewState)
{
	UpdateGoalDisplay();
}

void UCPCommonGoalWidget::UpdateGoalDisplay()
{
	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return;

	FName ActiveQuestID = QuestManager->GetActiveGoalQuestID();

	if (ActiveQuestID.IsNone())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ESlateVisibility::Visible);

	if (TextBlock_GoalTitle)
	{
		TextBlock_GoalTitle->SetText(QuestManager->GetQuestTitle(ActiveQuestID));
	}

	if (TextBlock_GoalText)
	{
		FText SummaryText = QuestManager->GetQuestSummaryText(ActiveQuestID);

		if (QuestManager->GetQuestCompletionType(ActiveQuestID) == EQuestCompletionType::ItemCollection)
		{
			TMap<FName, int32> HeldItemCounts;
			if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
			{
				if (UCPInventoryComponent* Inventory = PlayerPawn->FindComponentByClass<UCPInventoryComponent>())
				{
					HeldItemCounts = Inventory->GetItemCountsByID();
				}
			}

			FText ProgressText = QuestManager->GetItemProgressText(ActiveQuestID, HeldItemCounts);
			if (!ProgressText.IsEmpty())
			{
				SummaryText = FText::Format(FText::FromString(TEXT("{0}\n{1}")), SummaryText, ProgressText);
			}
		}

		TextBlock_GoalText->SetText(SummaryText);
	}
}

void UCPCommonGoalWidget::HandleInventoryUpdated()
{
	UpdateGoalDisplay();
}