// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Base/CPBaseFixedWidget.h"

void UCPBaseFixedWidget::ShowWidget()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UCPBaseFixedWidget::HideWidget()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

bool UCPBaseFixedWidget::IsWidgetShown() const
{
	return GetVisibility() == ESlateVisibility::Visible;
}
