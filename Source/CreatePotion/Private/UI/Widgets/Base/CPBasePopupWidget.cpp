// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"

void UCPBasePopupWidget::RequestClose()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
	}
	
	if (UCPUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UCPUIManagerSubsystem>())
	{
		UIManager->CloseWidget(this);
	}
}

void UCPBasePopupWidget::SetAutoClose(float Duration)
{
	if (Duration <= 0.f || !GetWorld()) return;
	
	GetWorld()->GetTimerManager().SetTimer(
		AutoCloseTimerHandle,
		this,
		&UCPBasePopupWidget::RequestClose,
		Duration,
		false
		);
}


