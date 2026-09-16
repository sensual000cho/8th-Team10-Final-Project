// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Quest/CPCommonGoalWidget.h"

#include "Components/TextBlock.h"
#include "Quest/QuestManager.h"

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
		TextBlock_GoalText->SetText(QuestManager->GetQuestSummaryText(ActiveQuestID));
	}
}