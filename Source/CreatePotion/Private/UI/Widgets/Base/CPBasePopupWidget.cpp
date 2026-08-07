// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"

void UCPBasePopupWidget::RequestClose()
{
	if (UCPUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UCPUIManagerSubsystem>())
	{
		UIManager->CloseWidget(this);
	}
}
