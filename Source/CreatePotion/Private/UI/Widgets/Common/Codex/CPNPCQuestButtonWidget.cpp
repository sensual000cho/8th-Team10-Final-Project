// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPNPCQuestButtonWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UCPNPCQuestButtonWidget::BindEvents()
{
	Super::BindEvents();
	
	if (Btn_Quest){
		Btn_Quest->OnClicked.AddDynamic(this, &UCPNPCQuestButtonWidget::HandleButtonClicked);
	}
}

void UCPNPCQuestButtonWidget::UnbindEvents()
{
	if (Btn_Quest){
		Btn_Quest->OnClicked.RemoveDynamic(this, &UCPNPCQuestButtonWidget::HandleButtonClicked);
	}
	Super::UnbindEvents();
}

void UCPNPCQuestButtonWidget::SetQuest(FName InQuestID, const FText& InQuestTitle)
{
	QuestID = InQuestID;
	if (Txt_QuestTitle){
		Txt_QuestTitle->SetText(InQuestTitle);
	}
}

void UCPNPCQuestButtonWidget::ApplyFocusState(FName FocusedQuestID)
{
	if (QuestID == FocusedQuestID){
		Txt_QuestTitle->SetColorAndOpacity(FLinearColor::Yellow);
	}else{
		Txt_QuestTitle->SetColorAndOpacity(FLinearColor::Black);
	}
}

void UCPNPCQuestButtonWidget::HandleButtonClicked()
{
	OnNPCQuestButtonClicked.Broadcast(QuestID);
}
