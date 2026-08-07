// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Notification/CPCommonNotificationWidget.h"
#include "Components/TextBlock.h"

void UCPCommonNotificationWidget::SetNotifiactionText(const FText& InText)
{
	if (TextBlock_Notification)
	{
		TextBlock_Notification->SetText(InText);
	}
}

void UCPCommonNotificationWidget::ShowWithAutoClose(const FText& InText, float Duration)
{
	SetNotifiactionText(InText);
	SetAutoClose(Duration);
}
