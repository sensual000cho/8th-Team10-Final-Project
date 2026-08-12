#include "UI/Widgets/Lab/TagChoice/CPTagRangeEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"

void UCPTagRangeEntryWidget::BindEvents()
{
	Super::BindEvents();
	if (Slider_Value)
	{
		Slider_Value->OnValueChanged.AddDynamic(this, &UCPTagRangeEntryWidget::OnSliderValueChanged);
	}
}

void UCPTagRangeEntryWidget::UnbindEvents()
{
	if (Slider_Value)
	{
		Slider_Value->OnValueChanged.RemoveDynamic(this, &UCPTagRangeEntryWidget::OnSliderValueChanged);
	}
	Super::UnbindEvents();
}

void UCPTagRangeEntryWidget::InitEntry(const FGameplayTag& InTag, int32 InitialValue)
{
	CurrentTag = InTag;

	if (Text_TagName)
	{
		FString DisplayStr = CurrentTag.GetTagName().ToString();
		DisplayStr.RemoveFromStart(TEXT("Alchemy."));
		Text_TagName->SetText(FText::FromString(DisplayStr));
	}

	if (Text_Feedback)
	{
		Text_Feedback->SetText(FText::GetEmpty());
	}

	if (Slider_Value)
	{
		Slider_Value->SetValue(static_cast<float>(InitialValue));
	}

	UpdateBlockColors(InitialValue);
}
void UCPTagRangeEntryWidget::OnSliderValueChanged(float NewValue)
{
	int32 IntVal = FMath::RoundToInt(NewValue);
	UpdateBlockColors(IntVal);
}

int32 UCPTagRangeEntryWidget::GetCurrentSliderValue() const
{
	if (Slider_Value)
	{
		return FMath::RoundToInt(Slider_Value->GetValue());
	}
	return 0;
}

void UCPTagRangeEntryWidget::SetFeedbackText(const FString& FeedbackStr)
{
	if (Text_Feedback)
	{
		Text_Feedback->SetText(FText::FromString(FeedbackStr));
	}
}

void UCPTagRangeEntryWidget::UpdateBlockColors(int32 Value)
{
	if (!HBox_Blocks) return;

	TArray<UWidget*> Blocks = HBox_Blocks->GetAllChildren();

	if (Blocks.Num() != 6)
	{
		return;
	}

	for (int32 i = 0; i < Blocks.Num(); ++i)
	{
		UBorder* BlockBorder = Cast<UBorder>(Blocks[i]);
		if (!BlockBorder) continue;

		UImage* BlockImage = Cast<UImage>(BlockBorder->GetContent());
		if (!BlockImage) continue;

		bool bIsActive = false;

		// [인덱스 구성]
		// 0: -3칸 | 1: -2칸 | 2: -1칸 | 3: +1칸 | 4: +2칸 | 5: +3칸
		if (Value > 0)
		{
			if (i >= 3 && i < 3 + Value)
			{
				bIsActive = true;
			}
		}
		else if (Value < 0)
		{
			if (i <= 2 && i >= 3 + Value)
			{
				bIsActive = true;
			}
		}

		if (bIsActive)
		{
			BlockImage->SetColorAndOpacity(ActiveColor);
		}
		else
		{
			BlockImage->SetColorAndOpacity(InactiveColor);
		}
	}
}