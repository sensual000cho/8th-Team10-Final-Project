// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPCodexItemListWidget.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"


void UCPCodexItemListWidget::NativeConstruct()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
	if (!CodexSubsystem) return;
	
	Super::NativeConstruct();
}

void UCPCodexItemListWidget::BindEvents()
{
	Super::BindEvents();
	CodexSubsystem->OnCodexItemSelected.AddDynamic(this, &UCPCodexItemListWidget::HandleCodexItemSelected);
}

void UCPCodexItemListWidget::HandleCodexItemSelected(UCPForageableItemData* ItemData)
{
	HideWidget();
}

