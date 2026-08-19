#include "UI/Widgets/Lab/TagChoice/CPTagRangeWidget.h"
#include "UI/Widgets/Lab/TagChoice/CPTagRangeEntryWidget.h"
#include "UI/Widgets/Lab/TagChoice/CPHintWidget.h"
#include "UI/Widgets/Lab/TagChoice/CPTagSelectionWidget.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameMode/CPLabGameMode.h"
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

TArray<FTagConfirmData> UCPTagRangeWidget::GetFinalizedTagValues() const
{
	TArray<FTagConfirmData> FinalizedValues;

	if (!ScrollBox_Entries) return FinalizedValues;

	UGameInstance* GI = GetGameInstance();
	UQuestManager* QuestManager = GI ? GI->GetSubsystem<UQuestManager>() : nullptr;
	FQuestAnswerData* Answer = nullptr;

	if (QuestManager && QuestManager->QuestAnswerTable)
	{
		Answer = QuestManager->QuestAnswerTable->FindRow<FQuestAnswerData>(CurrentQuestID, TEXT("Evaluation"));
	}

	TArray<UWidget*> ChildWidgets = ScrollBox_Entries->GetAllChildren();
	for (UWidget* Child : ChildWidgets)
	{
		UCPTagRangeEntryWidget* Entry = Cast<UCPTagRangeEntryWidget>(Child);
		if (!Entry) continue;

		FTagConfirmData Data;
		Data.Tag = Entry->GetEntryTag();
		Data.Value = Entry->GetCurrentSliderValue();
		Data.ResultStr = TEXT("?");

		//  태그 레벨(Min/Max 범위) 판정 폐기 - 태그 존재 유무로만 판정
		if (Answer)
		{
			const bool bIsRequested = Answer->RequestedEffects.ContainsByPredicate(
				[&](const FQuestEffectRequirement& Req) { return Req.Axis == Data.Tag; }
			);
			Data.ResultStr = bIsRequested ? TEXT("O") : TEXT("X");
		}


		FinalizedValues.Add(Data);
	}

	return FinalizedValues;
}

void UCPTagRangeWidget::OnEvaluateClicked()
{
	TArray<FTagConfirmData> EvaluatedData = GetFinalizedTagValues();

	TArray<UWidget*> ChildWidgets = ScrollBox_Entries->GetAllChildren();
	for (int32 i = 0; i < ChildWidgets.Num(); ++i)
	{
		if (UCPTagRangeEntryWidget* Entry = Cast<UCPTagRangeEntryWidget>(ChildWidgets[i]))
		{
			if (EvaluatedData.IsValidIndex(i))
			{
				Entry->SetFeedbackText(EvaluatedData[i].ResultStr);
			}
		}
	}
}

void UCPTagRangeWidget::OnConfirmClicked()
{
	// 사용자가 세팅한 최종 태그+수치 배열을 가져옴
	TArray<FTagConfirmData> FinalData = GetFinalizedTagValues();
	UE_LOG(LogTemp, Warning, TEXT("퀘스트 ID: %s"), *CurrentQuestID.ToString());
	UE_LOG(LogTemp, Warning, TEXT("확정된 태그 개수: %d개"), FinalData.Num());

	for (int32 i = 0; i < FinalData.Num(); ++i)
	{
		FString TagNameStr = FinalData[i].Tag.IsValid() ? FinalData[i].Tag.GetTagName().ToString() : TEXT("None");

		UE_LOG(LogTemp, Log, TEXT(" [%d] 태그: %s | 설정값: %2d | 판정: %s"),
			i,
			*TagNameStr,
			FinalData[i].Value,
			*FinalData[i].ResultStr);
	}
	
	UWorld* World = GetWorld();
	if (!World) return;

	ACPLabGameMode* LabGameMode = World->GetAuthGameMode<ACPLabGameMode>();
	if (!LabGameMode || !LabGameMode->AdvancePotionRequest()) return;
	
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
			if (UUserWidget* CreatedWidget = UIManager->PushWidget(TagSelectionWidgetClass))
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