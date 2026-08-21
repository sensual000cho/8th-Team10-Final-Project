// CPGridSlotWidgetBase.cpp

#include "UI/Widgets/Common/Container/CPGridSlotWidgetBase.h"
#include "GameFramework/InputSettings.h"			// 더블 클릭 입력 지연 시간을 사용하기 위해

#include "Character/CPPlayerController.h"
#include "UI/Context/CPContainerContextBase.h"

void UCPGridSlotWidgetBase::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DoubleClickCheckHandler);
	}
	Super::NativeDestruct();
}

FReply UCPGridSlotWidgetBase::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 소속 컨테이너가 없는 슬롯(초기화 안 됨)은 처리 불가
	if (!OwnerContainer)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	ACPPlayerController* PC = Cast<ACPPlayerController>(GetOwningPlayer());
	if (!PC || !PC->CurrentContextHandler)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	UCPContainerContextBase* ContextHandler = PC->CurrentContextHandler;
	bool bHandled = false;

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (InMouseEvent.IsLeftControlDown())      bHandled = ContextHandler->HandleCtrlLeftClick(this);
		else if (InMouseEvent.IsShiftDown())       bHandled = ContextHandler->HandleShiftLeftClick(this);
		else if (InMouseEvent.IsAltDown())         bHandled = ContextHandler->HandleAltLeftClick(this);
		else
		{
			float DoubleClickDelay = GetDefault<UInputSettings>()->DoubleClickTime;
			GetWorld()->GetTimerManager().SetTimer(
				DoubleClickCheckHandler, 
				this, 
				&UCPGridSlotWidgetBase::ExecuteLeftClickOnce, 
				DoubleClickDelay, 
				false
			);
			bHandled = true;
		}
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (InMouseEvent.IsLeftControlDown())      bHandled = ContextHandler->HandleCtrlRightClick(this);
		else if (InMouseEvent.IsShiftDown())       bHandled = ContextHandler->HandleShiftRightClick(this);
		else if (InMouseEvent.IsAltDown())         bHandled = ContextHandler->HandleAltRightClick(this);
		else                                       bHandled = ContextHandler->HandleRightClickOnly(this);
	}

	return bHandled ? FReply::Handled() : Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UCPGridSlotWidgetBase::ExecuteLeftClickOnce()
{
	if (ACPPlayerController* PC = Cast<ACPPlayerController>(GetOwningPlayer()))
	{
		if (PC->CurrentContextHandler)
		{
			PC->CurrentContextHandler->HandleLeftClickOnly(this);
		}
	}
}

void UCPGridSlotWidgetBase::CancelLeftClickCheckHandler()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DoubleClickCheckHandler);
	}
}
