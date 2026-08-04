// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Base/CPBaseUserWidget.h"

void UCPBaseUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindEvents();
}

void UCPBaseUserWidget::NativeDestruct()
{
	UnbindEvents();
	Super::NativeDestruct();
}

void UCPBaseUserWidget::BindEvents()
{
}

void UCPBaseUserWidget::UnbindEvents()
{
}
