#include "UI/Widgets/Lab/CPLabResultWidget.h"

#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "UI/Widgets/Lab/CPLabResultLabelValueRowWidget.h"
#include "Quest/QuestManager.h"
#include "UI/Widgets/Lab/Helper/CPLabResultUICalc.h"

void UCPLabResultWidget::ResetResultView()
{
	Text_ResultTitle->SetText(FText::GetEmpty());
	Text_Grade->SetText(FText::GetEmpty());
	Text_BaseReward->SetText(FText::GetEmpty());
	Text_FinalReward->SetText(FText::GetEmpty());

	VBox_Tips->ClearChildren();
}

void UCPLabResultWidget::SetHeaderText(const FText& InResultTitle, const FText& InGradeText)
{
	Text_ResultTitle->SetText(InResultTitle);
	Text_Grade->SetText(InGradeText);
}

void UCPLabResultWidget::SetRewardText(const FText& InBaseRewardText, const FText& InFinalRewardText)
{
	Text_BaseReward->SetText(InBaseRewardText);
	Text_FinalReward->SetText(InFinalRewardText);
}

void UCPLabResultWidget::AddTipRow(const FText& InReasonText, const FText& InAmountText)
{
	AddLabelValueRow(VBox_Tips, InReasonText, InAmountText);
}

void UCPLabResultWidget::UnbindEvents()
{
	if (UWorld* World = GetWorld()){
		World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
	}
	
	Super::UnbindEvents();
}

bool UCPLabResultWidget::InitializeResult(const FCPPotionDeliveryResult& DeliveryResult)
{
	if  (! FCPLabResultUICalc::ApplyDeliveryResult(DeliveryResult, this)) return false;
	
	UWorld* World = GetWorld();
	if (!World) return false;
		
	World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
	World->GetTimerManager().SetTimer(
		AutoCloseTimerHandle, this, &UCPLabResultWidget::RequestClose,	AutoCloseDelay,false);
	return true;
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
