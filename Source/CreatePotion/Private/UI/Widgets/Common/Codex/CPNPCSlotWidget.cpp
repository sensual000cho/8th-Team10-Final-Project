// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPNPCSlotWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"

void UCPNPCSlotWidget::BindEvents()
{
	Super::BindEvents();
	
	if(Btn_NPC){
		Btn_NPC->OnClicked.AddDynamic(this, &UCPNPCSlotWidget::HandleSlotClicked);
	}
}

void UCPNPCSlotWidget::UnbindEvents()
{
	if (Btn_NPC){
		Btn_NPC->OnClicked.RemoveDynamic(this, &UCPNPCSlotWidget::HandleSlotClicked);
	}
	Super::UnbindEvents();
}

void UCPNPCSlotWidget::SetNPCData(UCPNPCDataAsset* InNPCData)
{
	NPCData = InNPCData;
	Txt_NPCName->SetVisibility(ESlateVisibility::Visible);
	RefreshSlot();
}

void UCPNPCSlotWidget::HandleSlotClicked()
{
	if (!NPCData) return;
	
	UGameInstance* GI = GetGameInstance();
	UCPCodexSubsystem* CodexSubsystem = GI ? GI->GetSubsystem<UCPCodexSubsystem>() : nullptr;
	if (!CodexSubsystem) return;
	
	CodexSubsystem->OnCodexNPCSelected.Broadcast(NPCData);
}

void UCPNPCSlotWidget::RefreshSlot()
{
	if (!NPCData) return;
	if (Txt_NPCName){
		Txt_NPCName->SetText(FText::FromName(NPCData->NPCName));
	}
}
