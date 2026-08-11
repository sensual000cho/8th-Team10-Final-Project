#include "UI/Widgets/Lab/TagChoice/CPTagSelectionWidget.h"
#include "UI/Widgets/Lab/TagChoice/CPTagEntryWidget.h"
#include "UI/Widgets/Lab/TagChoice/CPTagRangeWidget.h"
#include "UI/Widgets/Lab/TagChoice/CPHintWidget.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameplayTagsManager.h"

void UCPTagSelectionWidget::BindEvents()
{
	Super::BindEvents();

	if (Button_Confirm) Button_Confirm->OnClicked.AddDynamic(this, &UCPTagSelectionWidget::OnConfirmClicked);

	if (Button_Slot1) Button_Slot1->OnClicked.AddDynamic(this, &UCPTagSelectionWidget::OnSlot1Clicked);
	if (Button_Slot2) Button_Slot2->OnClicked.AddDynamic(this, &UCPTagSelectionWidget::OnSlot2Clicked);
	if (Button_Slot3) Button_Slot3->OnClicked.AddDynamic(this, &UCPTagSelectionWidget::OnSlot3Clicked);
}

void UCPTagSelectionWidget::UnbindEvents()
{
	if (Button_Confirm) Button_Confirm->OnClicked.RemoveDynamic(this, &UCPTagSelectionWidget::OnConfirmClicked);

	if (Button_Slot1) Button_Slot1->OnClicked.RemoveDynamic(this, &UCPTagSelectionWidget::OnSlot1Clicked);
	if (Button_Slot2) Button_Slot2->OnClicked.RemoveDynamic(this, &UCPTagSelectionWidget::OnSlot2Clicked);
	if (Button_Slot3) Button_Slot3->OnClicked.RemoveDynamic(this, &UCPTagSelectionWidget::OnSlot3Clicked);

	Super::UnbindEvents();
}

void UCPTagSelectionWidget::InitTagSelectionWidget(FName InQuestID, const TArray<FGameplayTag>& InExistingSelectedTags, const TMap<FGameplayTag, int32>& InExistingSavedValues)
{
	CurrentQuestID = InQuestID;
	SavedTagValues = InExistingSavedValues;

	if (InExistingSelectedTags.Num() > 0)
	{
		SelectedTags = InExistingSelectedTags;
		if (SelectedTags.Num() < MaxSelectableTags)
		{
			int32 AddCount = MaxSelectableTags - SelectedTags.Num();
			for (int32 i = 0; i < AddCount; ++i)
			{
				SelectedTags.Add(FGameplayTag::EmptyTag);
			}
		}
	}
	else
	{
		SelectedTags.Init(FGameplayTag::EmptyTag, MaxSelectableTags);
	}

	if (UI_HintWidget)
	{
		UI_HintWidget->InitHintWidget(CurrentQuestID);
	}

	PopulateTagList();
	UpdateSelectedTagsUI();
}

void UCPTagSelectionWidget::PopulateTagList()
{
	if (!ScrollBox_TagList || !TagEntryWidgetClass) return;

	ScrollBox_TagList->ClearChildren();

	FGameplayTagContainer AllTags;
	UGameplayTagsManager::Get().RequestAllGameplayTags(AllTags, true);

	for (const FGameplayTag& Tag : AllTags)
	{
		FString TagString = Tag.GetTagName().ToString();

		if (TagString.StartsWith(TEXT("Alchemy")))
		{
			UCPTagEntryWidget* NewTagEntry = CreateWidget<UCPTagEntryWidget>(this, TagEntryWidgetClass);
			if (NewTagEntry)
			{
				NewTagEntry->InitTagEntry(Tag);
				NewTagEntry->OnTagSelected.AddDynamic(this, &UCPTagSelectionWidget::OnTagEntrySelected);

				ScrollBox_TagList->AddChild(NewTagEntry);
			}
		}
	}
}

void UCPTagSelectionWidget::OnTagEntrySelected(FGameplayTag SelectedTag)
{
	if (SelectedTags.Contains(SelectedTag))
	{
		UE_LOG(LogTemp, Log, TEXT("이미 선택된 태그입니다. 삭제는 우측 슬롯을 눌러주세요."));
		return;
	}
	for (int32 i = 0; i < MaxSelectableTags; ++i)
	{
		if (!SelectedTags[i].IsValid())
		{
			SelectedTags[i] = SelectedTag;
			UpdateSelectedTagsUI();
			return; 
		}
	}
}

void UCPTagSelectionWidget::UpdateSelectedTagsUI()
{
	TArray<UTextBlock*> SlotTexts = { Text_Slot1, Text_Slot2, Text_Slot3 };
	for (int32 i = 0; i < MaxSelectableTags; ++i)
	{
		if (SelectedTags[i].IsValid())
		{
			if (SlotTexts[i])
			{
				FString DisplayStr = SelectedTags[i].GetTagName().ToString();
				DisplayStr.RemoveFromStart(TEXT("Alchemy."));

				SlotTexts[i]->SetText(FText::FromString(DisplayStr));
			}
		}
		else 
		{
			if (SlotTexts[i])
			{
				SlotTexts[i]->SetText(FText::GetEmpty());
			}
		}
	}
}

void UCPTagSelectionWidget::OnSlot1Clicked()
{
	SelectedTags[0] = FGameplayTag::EmptyTag;
	UpdateSelectedTagsUI();
}

void UCPTagSelectionWidget::OnSlot2Clicked()
{
	SelectedTags[1] = FGameplayTag::EmptyTag;
	UpdateSelectedTagsUI();
}

void UCPTagSelectionWidget::OnSlot3Clicked()
{
	SelectedTags[2] = FGameplayTag::EmptyTag;
	UpdateSelectedTagsUI();
}

void UCPTagSelectionWidget::OnConfirmClicked()
{
	RequestClose();

	TArray<FGameplayTag> ValidTags;
	for (const FGameplayTag& Tag : SelectedTags)
	{
		if (Tag.IsValid())
		{
			ValidTags.Add(Tag);
		}
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>())
		{
			if (UUserWidget* CreatedWidget = UIManager->PushWidgetBP(TagRangeWidgetClass))
			{
				if (UCPTagRangeWidget* TagRangeWidget = Cast<UCPTagRangeWidget>(CreatedWidget))
				{
					TagRangeWidget->InitTagRangeWidget(CurrentQuestID, ValidTags, SavedTagValues);
				}
			}
		}
	}
}