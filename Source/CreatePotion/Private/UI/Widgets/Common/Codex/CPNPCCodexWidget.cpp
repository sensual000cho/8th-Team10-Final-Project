// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPNPCCodexWidget.h"

#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "Data/NPC/CPQuestNPCDataAsset.h"
#include "Internationalization/StringTable.h"
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
	
	FocusQuestID = NAME_None;
	
	UTexture2D* ImageTexture = CurrentNPCData->CodexImage.LoadSynchronous();
	Img_NPC->SetBrushFromTexture(ImageTexture);
	Txt_Name->SetText(FText::FromName(CurrentNPCData->NPCName));
	Txt_QuestTitle->SetText(FText::GetEmpty());
	Txt_QuestTitle->SetVisibility(ESlateVisibility::Collapsed);
	RefreshNPCDescription();
	
	RefreshCompletedQuestButtons();
}

void UCPNPCCodexWidget::RefreshCompletedQuestButtons()
{
	if (!SB_Quest) return;
	SB_Quest->ClearChildren();
	QuestButtons.Empty();
	
	const UCPQuestNPCDataAsset* QuestNPCData = Cast<UCPQuestNPCDataAsset>(CurrentNPCData);
	if (!QuestNPCData || !QuestButtonWidgetClass) return;
	
	const UGameInstance* GI = GetGameInstance();
	UQuestManager* QuestManager =GI ? GI->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return;
	
	TArray<FName> QuestIDs;
	QuestIDs.Append(QuestNPCData->QuestIDs);
	
	for (const FName& QuestID : QuestIDs){
		if (QuestID.IsNone() || QuestManager->GetQuestState(QuestID) != EQuestState::Completed) continue;
		
		UCPNPCQuestButtonWidget* QuestButton = CreateWidget<UCPNPCQuestButtonWidget>(this, QuestButtonWidgetClass);
		if (!QuestButton) continue;
		
		QuestButton->SetQuest(QuestID, QuestManager->GetQuestTitle(QuestID));
		QuestButton->ApplyFocusState(FocusQuestID);
		QuestButton->OnNPCQuestButtonClicked.AddDynamic(this, &UCPNPCCodexWidget::HandleQuestButtonClicked);
		SB_Quest->AddChild(QuestButton);
		QuestButtons.Add(QuestButton);
	}
}

void UCPNPCCodexWidget::RefreshNPCDescription()
{
	Txt_Description->SetText(FText::GetEmpty());
	
	if (!CurrentNPCData || !Txt_Description || !NPCStringTable) return;
	if (CurrentNPCData->CodexTextKey.IsNone()) return;
	
	const UGameInstance* GI = GetGameInstance();
	const UQuestManager* QuestManager = GI ? GI->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return;
	
	const int32 StoryStage = QuestManager->GetNPCStoryStage(CurrentNPCData->NPCName);
	const FString TextKeyString = FString::Printf(
		TEXT("%s.%02d"), *CurrentNPCData->CodexTextKey.ToString(), StoryStage);
	
	const FName TableId = NPCStringTable->GetStringTableId();
	Txt_Description->SetText(FText::FromStringTable(TableId, TextKeyString));
}

void UCPNPCCodexWidget::SetQuestDescription(FName QuestID)
{
	const UGameInstance* GI = GetGameInstance();
	UQuestManager* QuestManager =GI ? GI->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager || !Txt_Description) return;
	
	Txt_QuestTitle->SetText(QuestManager->GetQuestTitle(QuestID));
	Txt_QuestTitle->SetVisibility(ESlateVisibility::Visible);
	Txt_Description->SetText(QuestManager->GetCodexSummaryText(QuestID));
}

void UCPNPCCodexWidget::HandleQuestButtonClicked(FName QuestID)
{
	if (FocusQuestID == QuestID){
		FocusQuestID = NAME_None;
		
		// NPC Description으로 변경
		Txt_QuestTitle->SetText(FText::GetEmpty());
		Txt_QuestTitle->SetVisibility(ESlateVisibility::Collapsed);
		RefreshNPCDescription();
	}else{
		FocusQuestID = QuestID;
		SetQuestDescription(QuestID);	
	}
	
	for (UCPNPCQuestButtonWidget* QuestButton : QuestButtons){
		if (!QuestButton) continue;
		QuestButton->ApplyFocusState(FocusQuestID);
	}
}
