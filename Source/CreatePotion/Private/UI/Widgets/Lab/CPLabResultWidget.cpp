#include "UI/Widgets/Lab/CPLabResultWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#include "Components/CPInventoryComponent.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Quest/QuestManager.h"
#include "UI/Widgets/Lab/CPLabResultLabelValueRowWidget.h"
#include "UI/Widgets/Lab/Helper/CPLabResultUICalc.h"

void UCPLabResultWidget::ResetResultView()
{
	Text_ResultTitle->SetText(FText::GetEmpty());
	Text_Grade->SetText(FText::GetEmpty());
	Text_FinalReward->SetText(FText::GetEmpty());
}

void UCPLabResultWidget::SetHeaderText(const FText& InResultTitle, const FText& InGradeText)
{
	Text_ResultTitle->SetText(InResultTitle);
	Text_Grade->SetText(InGradeText);
	
	const FLinearColor GradeColor = InGradeText.EqualTo(FText::FromString(TEXT("성공"))) ? 
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("4F6B3AFF"))) : FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("963F32FF")));
	Text_Grade->SetColorAndOpacity(GradeColor);
}

void UCPLabResultWidget::SetRewardText(const FText& InBaseRewardText, const FText& InFinalRewardText)
{
	Text_FinalReward->SetText(InFinalRewardText);
}

bool UCPLabResultWidget::InitializeResult(FName QuestId)
{
	if (QuestId.IsNone()) return false;
	
	UWorld* World = GetWorld();
	if (!World) return false;
	
	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return false;
	
	const FCPQuestResult Result = QuestManager->GetResult(QuestId);
	if (!FCPLabResultUICalc::ApplyDeliveryResult(Result, this)) return false;
	
	// 퀘스트 완료 & 보상 있을 시 지급
	if (Result.DeliveryGrade == EDeliveryGrade::Perfect && Result.RewardAmount > 0){
		APlayerController* PlayerController = World->GetFirstPlayerController<APlayerController>();
		APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
		UCPInventoryComponent* Inventory = PlayerPawn ? PlayerPawn->FindComponentByClass<UCPInventoryComponent>() : nullptr;
		
		if (!Inventory || !Inventory->TryGetMoney(Result.RewardAmount)) return false;
	}
	
	if (CompletionSound && Result.DeliveryGrade == EDeliveryGrade::Perfect)
	{
		UGameplayStatics::PlaySound2D(this, CompletionSound);
	}
	QuestManager->ClearResult();
	
	World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
	World->GetTimerManager().SetTimer(
		AutoCloseTimerHandle, this, &UCPLabResultWidget::RequestClose,	AutoCloseDelay,false);
	return true;
}

void UCPLabResultWidget::UnbindEvents()
{
	if (UWorld* World = GetWorld()){
		World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
	}
	
	Super::UnbindEvents();
}

void UCPLabResultWidget::AddLabelValueRow(UVerticalBox* Container, const FText& LabelText, const FText& ValueText) const
{
	if (!LabelValueRowWidgetClass) return;

	UCPLabResultLabelValueRowWidget* Row = CreateWidget<UCPLabResultLabelValueRowWidget>(
		GetOwningPlayer(),
		LabelValueRowWidgetClass);
	if (!Row) return;

	Row->SetData(LabelText, ValueText);
	Container->AddChildToVerticalBox(Row);
}
