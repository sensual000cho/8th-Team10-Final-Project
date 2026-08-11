#include "UI/Widgets/Lab/TagChoice/CPTagRangeWidget.h"
#include "UI/Widgets/Lab/TagChoice/CPTagRangeEntryWidget.h"
#include "UI/Widgets/Lab/TagChoice/CPHintWidget.h"
#include "UI/Widgets/Lab/TagChoice/CPTagSelectionWidget.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Quest/QuestManager.h"

void UCPTagRangeWidget::BindEvents()
{
	Super::BindEvents();

	if (Button_Evaluate) Button_Evaluate->OnClicked.AddDynamic(this, &UCPTagRangeWidget::OnEvaluateClicked);
	if (Button_Confirm) Button_Confirm->OnClicked.AddDynamic(this, &UCPTagRangeWidget::OnConfirmClicked);
	if (Button_Back) Button_Back->OnClicked.AddDynamic(this, &UCPTagRangeWidget::OnBackClicked);
}

void UCPTagRangeWidget::UnbindEvents()
{
	if (Button_Evaluate) Button_Evaluate->OnClicked.RemoveDynamic(this, &UCPTagRangeWidget::OnEvaluateClicked);
	if (Button_Confirm) Button_Confirm->OnClicked.RemoveDynamic(this, &UCPTagRangeWidget::OnConfirmClicked);
	if (Button_Back) Button_Back->OnClicked.RemoveDynamic(this, &UCPTagRangeWidget::OnBackClicked);

	Super::UnbindEvents();
}

void UCPTagRangeWidget::InitTagRangeWidget(FName InQuestID, const TArray<FGameplayTag>& InSelectedTags, const TMap<FGameplayTag, int32>& InSavedValues)
{
	CurrentQuestID = InQuestID;
	SelectedTags = InSelectedTags;
	SavedTagValues = InSavedValues;

	if (UI_HintWidget)
	{
		UI_HintWidget->InitHintWidget(CurrentQuestID);
	}

	if (ScrollBox_Entries && EntryWidgetClass)
	{
		ScrollBox_Entries->ClearChildren();

		for (const FGameplayTag& Tag : SelectedTags)
		{
			UCPTagRangeEntryWidget* NewEntry = CreateWidget<UCPTagRangeEntryWidget>(this, EntryWidgetClass);
			if (NewEntry)
			{
				int32 InitialValue = SavedTagValues.Contains(Tag) ? SavedTagValues[Tag] : 0;
				NewEntry->InitEntry(Tag, InitialValue);
				ScrollBox_Entries->AddChild(NewEntry);
			}
		}
	}
}

TArray<FAlchemyProperty> UCPTagRangeWidget::GetFinalizedTagValues() const
{
	TArray<FAlchemyProperty> FinalizedValues;

	if (!ScrollBox_Entries) return FinalizedValues;

	TArray<UWidget*> ChildWidgets = ScrollBox_Entries->GetAllChildren();
	for (UWidget* Child : ChildWidgets)
	{
		UCPTagRangeEntryWidget* Entry = Cast<UCPTagRangeEntryWidget>(Child);
		if (!Entry) continue;

		FAlchemyProperty Prop;
		Prop.Tag = Entry->GetEntryTag();
		Prop.Value = Entry->GetCurrentSliderValue();

		FinalizedValues.Add(Prop);
	}

	return FinalizedValues;
}

void UCPTagRangeWidget::OnEvaluateClicked()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>();
	if (!QuestManager) return;

	if (!QuestManager->QuestAnswerTable) return;
	FQuestAnswerData* Answer = QuestManager->QuestAnswerTable->FindRow<FQuestAnswerData>(CurrentQuestID, TEXT("UI_Evaluation"));
	if (!Answer) return;

	TArray<UWidget*> ChildWidgets = ScrollBox_Entries->GetAllChildren();
	for (UWidget* Child : ChildWidgets)
	{
		UCPTagRangeEntryWidget* Entry = Cast<UCPTagRangeEntryWidget>(Child);
		if (!Entry) continue;

		FGameplayTag EntryTag = Entry->GetEntryTag();
		int32 EntryValue = Entry->GetCurrentSliderValue();

		const FQuestEffectRequirement* MatchingReq = Answer->RequestedEffects.FindByPredicate(
			[&](const FQuestEffectRequirement& Req) { return Req.Axis == EntryTag; }
		);

		if (!MatchingReq)
		{
			Entry->SetFeedbackText(TEXT("X"));
		}
		else if (EntryValue < MatchingReq->MinValue)
		{
			Entry->SetFeedbackText(TEXT("UP"));
		}
		else if (EntryValue > MatchingReq->MaxValue)
		{
			Entry->SetFeedbackText(TEXT("DOWN"));
		}
		else
		{
			Entry->SetFeedbackText(TEXT("O"));
		}
	}
}
void UCPTagRangeWidget::OnConfirmClicked()
{
	// 사용자가 세팅한 최종 태그+수치 배열을 가져옴
	TArray<FAlchemyProperty> FinalData = GetFinalizedTagValues();
	UE_LOG(LogTemp, Warning, TEXT("퀘스트 ID: %s"), *CurrentQuestID.ToString());
	UE_LOG(LogTemp, Warning, TEXT("확정된 태그 개수: %d개"), FinalData.Num());

	for (int32 i = 0; i < FinalData.Num(); ++i)
	{
		FString TagNameStr = FinalData[i].Tag.IsValid() ? FinalData[i].Tag.GetTagName().ToString() : TEXT("None");

		UE_LOG(LogTemp, Log, TEXT(" [%d] 태그: %s | 설정값: %d"),
			i,
			*TagNameStr,
			FinalData[i].Value);
	}
	//TODO : 재료 선택 UI 연결

	RequestClose();
}

void UCPTagRangeWidget::OnBackClicked()
{
	if (ScrollBox_Entries)
	{
		TArray<UWidget*> ChildWidgets = ScrollBox_Entries->GetAllChildren();
		for (UWidget* Child : ChildWidgets)
		{
			if (UCPTagRangeEntryWidget* Entry = Cast<UCPTagRangeEntryWidget>(Child))
			{
				SavedTagValues.Add(Entry->GetEntryTag(), Entry->GetCurrentSliderValue());
			}
		}
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>())
		{
			if (UUserWidget* CreatedWidget = UIManager->PushWidgetBP(TagSelectionWidgetClass))
			{
				if (UCPTagSelectionWidget* TagSelectionWidget = Cast<UCPTagSelectionWidget>(CreatedWidget))
				{
					TagSelectionWidget->InitTagSelectionWidget(CurrentQuestID, SelectedTags, SavedTagValues);
				}
			}
		}
	}

	RequestClose();
}