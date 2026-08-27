// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"

void UCPBaseUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindEvents();
	CachedUIManager = GetGameInstance()->GetSubsystem<UCPUIManagerSubsystem>();
	
	if (CachedUIManager && OpenSound)
	{
		CachedUIManager->PlayWidgetSound(OpenSound);
	}
}

void UCPBaseUserWidget::NativeDestruct()
{
	if (CachedUIManager && OpenSound)
	{
		CachedUIManager->PlayWidgetSound(CloseSound);
	}
	
	UnbindEvents();
	Super::NativeDestruct();
}

void UCPBaseUserWidget::BindEvents()
{
	
}

void UCPBaseUserWidget::UnbindEvents()
{
	
}
