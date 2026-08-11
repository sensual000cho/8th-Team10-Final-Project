#include "UI/Widgets/Lab/CPLabResultWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

namespace
{
	FText MakeText(const TCHAR* Value)
	{
		return FText::FromString(Value);
	}

	void ConfigureRowText(UTextBlock* TextBlock, const FText& Text, ETextJustify::Type Justification)
	{
		if (!TextBlock) return;

		TextBlock->SetText(Text);
		TextBlock->SetJustification(Justification);
		TextBlock->SetAutoWrapText(true);

		FSlateFontInfo Font = TextBlock->GetFont();
		Font.Size = 16;
		TextBlock->SetFont(Font);
	}

	void SetFill(UHorizontalBoxSlot* Slot)
	{
		if (!Slot) return;

		FSlateChildSize Size;
		Size.SizeRule = ESlateSizeRule::Fill;
		Slot->SetSize(Size);
	}

	void SetAuto(UHorizontalBoxSlot* Slot)
	{
		if (!Slot) return;

		FSlateChildSize Size;
		Size.SizeRule = ESlateSizeRule::Automatic;
		Slot->SetSize(Size);
	}
}

void UCPLabResultWidget::SetResultViewData(const FCPLabResultViewData& InViewData)
{
	bUseMockData = false;
	CurrentViewData = InViewData;
	RefreshView();
}

void UCPLabResultWidget::ShowMockOutcome(ECPLabResultPresentationOutcome InOutcome)
{
	bUseMockData = true;
	PreviewOutcome = InOutcome;
	CurrentViewData = MakeMockData(InOutcome);
	RefreshView();
}

void UCPLabResultWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (bUseMockData)
	{
		CurrentViewData = MakeMockData(PreviewOutcome);
	}

	RefreshView();
}

void UCPLabResultWidget::BindEvents()
{
	Super::BindEvents();

	if (Button_Primary)
	{
		Button_Primary->OnClicked.AddUniqueDynamic(this, &UCPLabResultWidget::HandlePrimaryAction);
	}

	if (Button_Secondary)
	{
		Button_Secondary->OnClicked.AddUniqueDynamic(this, &UCPLabResultWidget::HandleSecondaryAction);
	}
}

void UCPLabResultWidget::UnbindEvents()
{
	if (Button_Primary)
	{
		Button_Primary->OnClicked.RemoveDynamic(this, &UCPLabResultWidget::HandlePrimaryAction);
	}

	if (Button_Secondary)
	{
		Button_Secondary->OnClicked.RemoveDynamic(this, &UCPLabResultWidget::HandleSecondaryAction);
	}

	Super::UnbindEvents();
}

void UCPLabResultWidget::HandlePrimaryAction()
{
	if (CurrentViewData.Outcome == ECPLabResultPresentationOutcome::Success)
	{
		OnConfirmRequested.Broadcast();
		return;
	}

	OnRetryRequested.Broadcast();
}

void UCPLabResultWidget::HandleSecondaryAction()
{
	if (CurrentViewData.Outcome != ECPLabResultPresentationOutcome::Success)
	{
		OnContinueRequested.Broadcast();
	}
}

void UCPLabResultWidget::RefreshView()
{
	if (Text_ResultTitle) Text_ResultTitle->SetText(CurrentViewData.ResultTitle);
	if (Text_Grade) Text_Grade->SetText(CurrentViewData.GradeText);
	if (Text_Diagnosis) Text_Diagnosis->SetText(CurrentViewData.DiagnosisText);
	if (Text_BaseReward) Text_BaseReward->SetText(CurrentViewData.BaseRewardText);
	if (Text_FinalReward) Text_FinalReward->SetText(CurrentViewData.FinalRewardText);

	RebuildEffectRows();
	RebuildConditionRows();
	RebuildTipRows();

	const bool bIsSuccess = CurrentViewData.Outcome == ECPLabResultPresentationOutcome::Success;
	if (Button_Secondary)
	{
		Button_Secondary->SetVisibility(bIsSuccess ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	if (Text_PrimaryAction)
	{
		Text_PrimaryAction->SetText(bIsSuccess ? MakeText(TEXT("확인")) : MakeText(TEXT("재시도")));
	}

	if (Text_SecondaryAction)
	{
		Text_SecondaryAction->SetText(MakeText(TEXT("넘어가기")));
	}
}

void UCPLabResultWidget::RebuildEffectRows()
{
	if (!VBox_Effects) return;

	VBox_Effects->ClearChildren();
	for (const FCPLabResultEffectView& Effect : CurrentViewData.Effects)
	{
		AddTwoColumnRow(VBox_Effects, Effect.EffectName, Effect.ValueText);
	}
}

void UCPLabResultWidget::RebuildConditionRows()
{
	if (!VBox_Conditions) return;

	VBox_Conditions->ClearChildren();
	for (const FCPLabResultConditionView& Condition : CurrentViewData.Conditions)
	{
		AddConditionRow(Condition);
	}
}

void UCPLabResultWidget::RebuildTipRows()
{
	if (!VBox_Tips) return;

	VBox_Tips->ClearChildren();
	for (const FCPLabResultTipView& TipItem : CurrentViewData.TipItems)
	{
		AddTwoColumnRow(VBox_Tips, TipItem.ReasonText, TipItem.AmountText);
	}
}

void UCPLabResultWidget::AddTwoColumnRow(
	UVerticalBox* Container,
	const FText& LeftText,
	const FText& RightText) const
{
	if (!Container || !WidgetTree) return;

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UTextBlock* Left = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	UTextBlock* Right = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());

	ConfigureRowText(Left, LeftText, ETextJustify::Left);
	ConfigureRowText(Right, RightText, ETextJustify::Right);

	UHorizontalBoxSlot* LeftSlot = Row->AddChildToHorizontalBox(Left);
	SetFill(LeftSlot);
	LeftSlot->SetPadding(FMargin(8.f, 5.f));

	UHorizontalBoxSlot* RightSlot = Row->AddChildToHorizontalBox(Right);
	SetAuto(RightSlot);
	RightSlot->SetPadding(FMargin(8.f, 5.f));

	Container->AddChildToVerticalBox(Row);
}

void UCPLabResultWidget::AddConditionRow(const FCPLabResultConditionView& Condition) const
{
	if (!VBox_Conditions || !WidgetTree) return;

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UTextBlock* Name = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	UTextBlock* Actual = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	UTextBlock* Target = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	UTextBlock* Status = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());

	ConfigureRowText(Name, Condition.ConditionName, ETextJustify::Left);
	ConfigureRowText(Actual, Condition.ActualValueText, ETextJustify::Center);
	ConfigureRowText(
		Target,
		Condition.bShowTargetValue ? Condition.TargetValueText : MakeText(TEXT("비공개")),
		ETextJustify::Center);
	ConfigureRowText(Status, Condition.StatusText, ETextJustify::Center);

	UHorizontalBoxSlot* NameSlot = Row->AddChildToHorizontalBox(Name);
	SetFill(NameSlot);
	NameSlot->SetPadding(FMargin(8.f, 5.f));

	for (UTextBlock* ValueText : {Actual, Target, Status})
	{
		UHorizontalBoxSlot* RowSlot = Row->AddChildToHorizontalBox(ValueText);
		SetAuto(RowSlot);
		RowSlot->SetPadding(FMargin(8.f, 5.f));
	}

	VBox_Conditions->AddChildToVerticalBox(Row);
}

FCPLabResultViewData UCPLabResultWidget::MakeMockData(ECPLabResultPresentationOutcome Outcome)
{
	FCPLabResultViewData Data;
	Data.Outcome = Outcome;
	Data.ResultTitle = MakeText(TEXT("포션 제조 결과"));

	Data.Effects =
	{
		{MakeText(TEXT("잠이 잘 옴")), MakeText(TEXT("+2"))},
		{MakeText(TEXT("몸이 따뜻해짐")), MakeText(TEXT("+1"))},
		{MakeText(TEXT("마음이 편안해짐")), MakeText(TEXT("+1"))},
	};

	Data.Conditions =
	{
		{MakeText(TEXT("수면 유도")), MakeText(TEXT("+2")), MakeText(TEXT("Lv.2 이상")), MakeText(TEXT("충족")), true},
		{MakeText(TEXT("체온 상승")), MakeText(TEXT("+1")), MakeText(TEXT("Lv.1 이상")), MakeText(TEXT("충족")), true},
		{MakeText(TEXT("정서 안정")), MakeText(TEXT("+1")), MakeText(TEXT("Lv.1 이상")), MakeText(TEXT("충족")), true},
	};

	switch (Outcome)
	{
	case ECPLabResultPresentationOutcome::Success:
		Data.GradeText = MakeText(TEXT("성공"));
		Data.DiagnosisText = MakeText(TEXT("요청한 효능이 모두 확인되었습니다."));
		Data.TipItems =
		{
			{MakeText(TEXT("신속한 제조")), MakeText(TEXT("+24 G"))},
			{MakeText(TEXT("힌트 미사용")), MakeText(TEXT("+12 G"))},
		};
		Data.BaseRewardText = MakeText(TEXT("120 G"));
		Data.FinalRewardText = MakeText(TEXT("156 G"));
		break;

	case ECPLabResultPresentationOutcome::PartialSuccess:
		Data.GradeText = MakeText(TEXT("부분 성공 (임시)"));
		Data.Effects[1].ValueText = MakeText(TEXT("0"));
		Data.Conditions[1].ActualValueText = MakeText(TEXT("0"));
		Data.Conditions[1].StatusText = MakeText(TEXT("부족"));
		Data.DiagnosisText = MakeText(TEXT("체온 관련 효능이 부족합니다. 공개 수준은 임시입니다."));
		Data.BaseRewardText = MakeText(TEXT("80 G"));
		Data.FinalRewardText = MakeText(TEXT("80 G"));
		break;

	case ECPLabResultPresentationOutcome::Failure:
		Data.GradeText = MakeText(TEXT("실패 (임시)"));
		Data.Effects[0].ValueText = MakeText(TEXT("0"));
		Data.Effects[1].ValueText = MakeText(TEXT("-1"));
		Data.Conditions[0].ActualValueText = MakeText(TEXT("0"));
		Data.Conditions[0].StatusText = MakeText(TEXT("부족"));
		Data.Conditions[1].ActualValueText = MakeText(TEXT("-1"));
		Data.Conditions[1].StatusText = MakeText(TEXT("부족"));
		Data.DiagnosisText = MakeText(TEXT("요청과 맞지 않는 결과입니다. 재료 또는 가공 순서를 다시 확인하세요."));
		Data.BaseRewardText = MakeText(TEXT("0 G"));
		Data.FinalRewardText = MakeText(TEXT("0 G"));
		break;

	default:
		break;
	}

	return Data;
}
