// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Quest/CPQuestSelectWidget.h"

#include "Components/Button.h"

void UCPQuestSelectWidget::BindEvents()
{
	Super::BindEvents();
	
	UE_LOG(LogTemp, Warning, TEXT("[QuestSelectWidget] 퀘스트 선택 창 BindEvents 실행됨"));
	Button_CallCustomer->OnClicked.AddDynamic(this, &UCPQuestSelectWidget::HandleCallButtonClicked);
	Button_Cancel->OnClicked.AddDynamic(this, &UCPQuestSelectWidget::HandleCancelButtonClicked);
}

void UCPQuestSelectWidget::UnbindEvents()
{
	Button_CallCustomer->OnClicked.RemoveDynamic(this, &UCPQuestSelectWidget::HandleCallButtonClicked);
	Button_Cancel->OnClicked.RemoveDynamic(this, &UCPQuestSelectWidget::HandleCancelButtonClicked);
	Super::UnbindEvents();
}

void UCPQuestSelectWidget::HandleCallButtonClicked()
{
	// TODO: NPC 호출 기능 추가
}

void UCPQuestSelectWidget::HandleCancelButtonClicked()
{
	CachedUIManager->CloseWidget(this);
}
