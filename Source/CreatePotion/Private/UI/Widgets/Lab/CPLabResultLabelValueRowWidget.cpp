#include "UI/Widgets/Lab/CPLabResultLabelValueRowWidget.h"

#include "Components/TextBlock.h"

void UCPLabResultLabelValueRowWidget::SetData(const FText& InLabelText, const FText& InValueText)
{
	Text_Label->SetText(InLabelText);
	Text_Value->SetText(InValueText);
}
