// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Notification/CPCommonNotificationWidget.h"
#include "Components/TextBlock.h"

void UCPCommonNotificationWidget::SetNotificationText(const FText& InText)
{
	if (TextBlock_Notification)
	{
		TextBlock_Notification->SetText(InText);
	}
}

void UCPCommonNotificationWidget::ShowNotification()
{
	AddToViewport();
	SetAutoClose(NotificationDuration);
}

void UCPCommonNotificationWidget::RequestClose()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
	}
	RemoveFromParent();
}
