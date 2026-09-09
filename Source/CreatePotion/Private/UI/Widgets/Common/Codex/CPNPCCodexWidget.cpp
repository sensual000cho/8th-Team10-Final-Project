// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPNPCCodexWidget.h"

#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "Data/NPC/CPQuestNPCDataAsset.h"
#include "Quest/QuestManager.h"
#include "UI/Widgets/Common/Codex/CPNPCQuestButtonWidget.h"

void UCPNPCCodexWidget::SetNPCData(UCPNPCDataAsset* InNPCData)
{
	CurrentNPCData = InNPCData;
	RefreshCodex();
}

void UCPNPCCodexWidget::RefreshCodex()
{
	if (!CurrentNPCData) return;
	
	if (Img_NPC){
		UTexture2D* ImageTexture = CurrentNPCData->CodexImage.LoadSynchronous();
		Img_NPC->SetBrushFromTexture(ImageTexture);
	}
	
	if (Txt_Name){
		Txt_Name->SetText(FText::FromName(CurrentNPCData->NPCName));
	}
	
	if (Txt_Description){
		Txt_Description->SetText(FText::GetEmpty());
	}
	
	RefreshCompletedQuestButtons();
}

void UCPNPCCodexWidget::RefreshCompletedQuestButtons()
{
	if (!SB_Quest) return;
	SB_Quest->ClearChildren();
	
	const UCPQuestNPCDataAsset* QuestNPCData = Cast<UCPQuestNPCDataAsset>(CurrentNPCData);
	if (!QuestNPCData || !QuestButtonWidgetClass) return;
	
	const UGameInstance* GI = GetGameInstance();
	UQuestManager* QuestManager =GI ? GI->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return;
	
	TArray<FName> QuestIDs;
	QuestIDs.Append(QuestNPCData->TownQuestIDs);
	QuestIDs.Append(QuestNPCData->LabQuestIDs);
	
	for (const FName& QuestID : QuestIDs){
		if (QuestID.IsNone() || QuestManager->GetQuestState(QuestID) != EQuestState::Completed) continue;
		
		UCPNPCQuestButtonWidget* QuestButton = CreateWidget<UCPNPCQuestButtonWidget>(this, QuestButtonWidgetClass);
		if (!QuestButton) continue;
		
		QuestButton->SetQuest(QuestID, QuestManager->GetQuestTitle(QuestID));
		QuestButton->OnNPCQuestButtonClicked.AddDynamic(this, &UCPNPCCodexWidget::HandleQuestButtonClicked);
		SB_Quest->AddChild(QuestButton);
	}
}

void UCPNPCCodexWidget::SetQuestDsecription(FName QuestID)
{
	const UGameInstance* GI = GetGameInstance();
	UQuestManager* QuestManager =GI ? GI->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager || !Txt_Description) return;
	
	Txt_Description->SetText(QuestManager->GetQuestSummaryText(QuestID));
}

void UCPNPCCodexWidget::HandleQuestButtonClicked(FName QuestID)
{
	SetQuestDsecription(QuestID);
}
