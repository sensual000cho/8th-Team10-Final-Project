// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"

void UCPBaseUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindEvents();
	CachedUIManager = GetGameInstance()->GetSubsystem<UCPUIManagerSubsystem>();
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
