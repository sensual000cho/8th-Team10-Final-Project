#include "UI/Widgets/Lab/CPLabResultConditionRowWidget.h"

#include "Components/TextBlock.h"

void UCPLabResultConditionRowWidget::SetData(
	const FText& InConditionName,
	const FText& InActualValueText,
	const FText& InTargetValueText,
	const FText& InStatusText)
{
	Text_ConditionName->SetText(InConditionName);
	Text_ActualValue->SetText(InActualValueText);
	Text_TargetValue->SetText(InTargetValueText);
	Text_Status->SetText(InStatusText);
}
