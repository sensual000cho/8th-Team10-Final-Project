#include "UI/Widgets/Lab/TagChoice/CPHintWidget.h" 
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Quest/QuestManager.h"

void UCPHintWidget::BindEvents()
{
	Super::BindEvents();

	if (Button_AskHint)
	{
		Button_AskHint->OnClicked.AddDynamic(this, &UCPHintWidget::OnAskHintClicked);
	}
}

void UCPHintWidget::UnbindEvents()
{
	if (Button_AskHint)
	{
		Button_AskHint->OnClicked.RemoveDynamic(this, &UCPHintWidget::OnAskHintClicked);
	}

	Super::UnbindEvents();
}

void UCPHintWidget::InitHintWidget(FName InQuestID)
{
	CurrentQuestID = InQuestID;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>())
		{
			CurrentHintLevel = QuestManager->GetQuestHintLevel(CurrentQuestID);
		}
	}

	UpdateHintUI();
}

void UCPHintWidget::UpdateHintUI()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>();
	if (!QuestManager || CurrentQuestID.IsNone()) return;

	FText HintText;
	if (CurrentHintLevel == 0) HintText = QuestManager->GetSessionHintText(CurrentQuestID);
	else if (CurrentHintLevel == 1) HintText = QuestManager->GetSessionHintTextDetailed(CurrentQuestID);
	else HintText = QuestManager->GetSessionHintTextDetailed2(CurrentQuestID);

	if (Text_Hint) Text_Hint->SetText(HintText);

	if (Button_AskHint)
	{
		if (CurrentHintLevel < 2)
		{
			Button_AskHint->SetVisibility(ESlateVisibility::Visible);
			Button_AskHint->SetIsEnabled(true);
		}
		else
		{
			Button_AskHint->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UCPHintWidget::OnAskHintClicked()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>())
		{
			CurrentHintLevel++;
			QuestManager->SetQuestHintLevel(CurrentQuestID, CurrentHintLevel);
			UpdateHintUI();
		}
	}
}