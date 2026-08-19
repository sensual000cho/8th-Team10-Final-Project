// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Quest/CPQuestSelectWidget.h"

#include "Components/Button.h"

void UCPQuestSelectWidget::BindEvents()
{
	Super::BindEvents();
	
	Button_CallCustomer->OnClicked.AddDynamic(this, &UCPQuestSelectWidget::HandleCallButtonClicked);
	Button_Cancel->OnClicked.AddDynamic(this, &UCPQuestSelectWidget::HandleCancelButtonClicked);
}

void UCPQuestSelectWidget::UnbindEvents()
{
	Super::UnbindEvents();
	
	Button_CallCustomer->OnClicked.RemoveDynamic(this, &UCPQuestSelectWidget::HandleCallButtonClicked);
	Button_Cancel->OnClicked.RemoveDynamic(this, &UCPQuestSelectWidget::HandleCancelButtonClicked);
}

void UCPQuestSelectWidget::HandleCallButtonClicked()
{
	// TODO: NPC 호출 기능 추가
}

void UCPQuestSelectWidget::HandleCancelButtonClicked()
{
	// TODO: UI 닫기 
	CachedUIManager->CloseWidget(this);
}
