// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPCodexMainWidget.h"

#include "Components/WidgetSwitcher.h"
#include "UI/Widgets/Common/Codex/CPForageableCodexWidget.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "UI/Widgets/Common/Codex/CPNPCCodexWidget.h"

void UCPCodexMainWidget::BindEvents()
{
	Super::BindEvents();
	
	if (!CodexSubsystem)
	{
		UGameInstance* GI = GetGameInstance();
		if (!GI) return;
		CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
	}
	
	CodexSubsystem->OnCodexItemSelected.AddUniqueDynamic(this, &UCPCodexMainWidget::OpenItemDetail);
	CodexSubsystem->OnCodexNPCSelected.AddUniqueDynamic(this, &UCPCodexMainWidget::OpenNPCDetail);
}

void UCPCodexMainWidget::UnbindEvents()
{
	if (CodexSubsystem)
	{
		CodexSubsystem->OnCodexItemSelected.RemoveDynamic(this, &UCPCodexMainWidget::OpenItemDetail);	
		CodexSubsystem->OnCodexNPCSelected.RemoveDynamic(this, &UCPCodexMainWidget::OpenNPCDetail);	
	}
}

void UCPCodexMainWidget::OpenItemDetail(UCPForageableItemData* SelectedItemData)
{
	CachedUIManager->PlayWidgetSound(OpenSound);
	WBP_CPForageableCodex->SetForageableItem(SelectedItemData);
	WidgetSwitcher->SetActiveWidgetIndex(2);
}

void UCPCodexMainWidget::OpenNPCDetail(UCPNPCDataAsset* SelectedNPCData)
{
	CachedUIManager->PlayWidgetSound(OpenSound);
	WBP_CPNPCCodex->SetNPCData(SelectedNPCData);
	WidgetSwitcher->SetActiveWidgetIndex(3);
}
