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
	AutoCloseRemaining = Duration > 0.f ? Duration : -1.f;
}

bool UCPBasePopupWidget::HandleEscape()
{
	RequestClose();
	return true;
}

void UCPBasePopupWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (AutoCloseRemaining > 0.f)
	{
		AutoCloseRemaining -= InDeltaTime;
		if (AutoCloseRemaining <=0.f)
		{
			AutoCloseRemaining = -1.f;
			RequestClose();
		}
	}
}


