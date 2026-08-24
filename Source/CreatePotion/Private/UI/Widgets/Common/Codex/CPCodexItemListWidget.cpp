// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPCodexItemListWidget.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"

void UCPCodexItemListWidget::BindEvents()
{
	Super::BindEvents();
	CodexItemGrid->OnCodexGridItemClicked.AddDynamic(this, &UCPCodexItemListWidget::HandleSlotClicked);
}

void UCPCodexItemListWidget::HandleSlotClicked(const FCPForageableCodexEntry& CodexEntry)
{
	// TODO: 해당 아이템 WBP 열기
	
}


