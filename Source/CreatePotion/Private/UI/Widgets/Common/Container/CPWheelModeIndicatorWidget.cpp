// CPWheelModeIndicatorWidget.cpp

#include "UI/Widgets/Common/Container/CPWheelModeIndicatorWidget.h"
#include "Components/TextBlock.h"

#include "Types/CPInputModeTypes.h"

void UCPWheelModeIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Collapsed); // 처음, 평소엔 Collapsed 처리
}

void UCPWheelModeIndicatorWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(HideTimerHandler);
	Super::NativeDestruct();
}

void UCPWheelModeIndicatorWidget::ShowMode(EWheelInputMode NewMode)
{
	if (!WheelModeText)
	{
		return;
	}

	FText DisplayText;
	switch (NewMode)
	{
	case EWheelInputMode::CameraZoom:       DisplayText = FText::FromString(TEXT("카메라 줌")); break;
	case EWheelInputMode::PotionSelect:     DisplayText = FText::FromString(TEXT("포션 선택")); break;
	case EWheelInputMode::IngredientSelect: DisplayText = FText::FromString(TEXT("재료 선택")); break;
	}

	WheelModeText->SetText(DisplayText);
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	GetWorld()->GetTimerManager().ClearTimer(HideTimerHandler);
	GetWorld()->GetTimerManager().SetTimer(HideTimerHandler,
		this, &UCPWheelModeIndicatorWidget::HideIndicator, DisplayDuration, false);
}

void UCPWheelModeIndicatorWidget::HideIndicator()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
