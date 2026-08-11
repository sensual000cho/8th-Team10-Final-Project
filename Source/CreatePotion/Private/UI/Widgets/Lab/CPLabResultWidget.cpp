#include "UI/Widgets/Lab/CPLabResultWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "UI/Widgets/Lab/CPLabResultConditionRowWidget.h"
#include "UI/Widgets/Lab/CPLabResultLabelValueRowWidget.h"

void UCPLabResultWidget::ResetResultView()
{
	Text_ResultTitle->SetText(FText::GetEmpty());
	Text_Grade->SetText(FText::GetEmpty());
	Text_Diagnosis->SetText(FText::GetEmpty());
	Text_BaseReward->SetText(FText::GetEmpty());
	Text_FinalReward->SetText(FText::GetEmpty());

	VBox_Effects->ClearChildren();
	VBox_Conditions->ClearChildren();
	VBox_Tips->ClearChildren();

	ShowConfirmAction();
}

void UCPLabResultWidget::SetHeaderText(const FText& InResultTitle, const FText& InGradeText)
{
	Text_ResultTitle->SetText(InResultTitle);
	Text_Grade->SetText(InGradeText);
}

void UCPLabResultWidget::SetDiagnosisText(const FText& InDiagnosisText)
{
	Text_Diagnosis->SetText(InDiagnosisText);
}

void UCPLabResultWidget::SetRewardText(const FText& InBaseRewardText, const FText& InFinalRewardText)
{
	Text_BaseReward->SetText(InBaseRewardText);
	Text_FinalReward->SetText(InFinalRewardText);
}

void UCPLabResultWidget::AddEffectRow(const FText& InEffectName, const FText& InValueText)
{
	AddLabelValueRow(VBox_Effects, InEffectName, InValueText);
}

void UCPLabResultWidget::AddConditionRow(
	const FText& InConditionName,
	const FText& InActualValueText,
	const FText& InTargetValueText,
	const FText& InStatusText)
{
	if (!ConditionRowWidgetClass) return;

	UCPLabResultConditionRowWidget* Row = CreateWidget<UCPLabResultConditionRowWidget>(
		GetOwningPlayer(),
		ConditionRowWidgetClass);
	if (!Row) return;

	Row->SetData(InConditionName, InActualValueText, InTargetValueText, InStatusText);
	VBox_Conditions->AddChildToVerticalBox(Row);
}

void UCPLabResultWidget::AddTipRow(const FText& InReasonText, const FText& InAmountText)
{
	AddLabelValueRow(VBox_Tips, InReasonText, InAmountText);
}

void UCPLabResultWidget::ShowConfirmAction()
{
	bPrimaryActionIsConfirm = true;
	Text_PrimaryAction->SetText(FText::FromString(TEXT("확인")));
	Button_Secondary->SetVisibility(ESlateVisibility::Collapsed);
}

void UCPLabResultWidget::ShowRetryAndContinueActions()
{
	bPrimaryActionIsConfirm = false;
	Text_PrimaryAction->SetText(FText::FromString(TEXT("재시도")));
	Text_SecondaryAction->SetText(FText::FromString(TEXT("넘어가기")));
	Button_Secondary->SetVisibility(ESlateVisibility::Visible);
}

void UCPLabResultWidget::BindEvents()
{
	Super::BindEvents();
	Button_Primary->OnClicked.AddUniqueDynamic(this, &UCPLabResultWidget::HandlePrimaryAction);
	Button_Secondary->OnClicked.AddUniqueDynamic(this, &UCPLabResultWidget::HandleSecondaryAction);
}

void UCPLabResultWidget::UnbindEvents()
{
	Button_Primary->OnClicked.RemoveDynamic(this, &UCPLabResultWidget::HandlePrimaryAction);
	Button_Secondary->OnClicked.RemoveDynamic(this, &UCPLabResultWidget::HandleSecondaryAction);
	Super::UnbindEvents();
}

void UCPLabResultWidget::HandlePrimaryAction()
{
	if (bPrimaryActionIsConfirm)
	{
		OnConfirmRequested.Broadcast();
		return;
	}

	OnRetryRequested.Broadcast();
}

void UCPLabResultWidget::HandleSecondaryAction()
{
	if (!bPrimaryActionIsConfirm)
	{
		OnContinueRequested.Broadcast();
	}
}

void UCPLabResultWidget::AddLabelValueRow(
	UVerticalBox* Container,
	const FText& LabelText,
	const FText& ValueText) const
{
	if (!LabelValueRowWidgetClass) return;

	UCPLabResultLabelValueRowWidget* Row = CreateWidget<UCPLabResultLabelValueRowWidget>(
		GetOwningPlayer(),
		LabelValueRowWidgetClass);
	if (!Row) return;

	Row->SetData(LabelText, ValueText);
	Container->AddChildToVerticalBox(Row);
}
