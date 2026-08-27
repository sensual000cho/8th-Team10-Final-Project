// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPCodexMainWidget.h"

#include "Components/WidgetSwitcher.h"
#include "UI/Widgets/Common/Codex/CPForageableCodexWidget.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"

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
}

void UCPCodexMainWidget::UnbindEvents()
{
	if (CodexSubsystem)
	{
		CodexSubsystem->OnCodexItemSelected.RemoveDynamic(this, &UCPCodexMainWidget::OpenItemDetail);	
	}
}

void UCPCodexMainWidget::OpenItemDetail(UCPForageableItemData* SelectedItemData)
{
	CachedUIManager->PlayWidgetSound(OpenSound);
	WBP_CPForageableCodex->SetForageableItem(SelectedItemData);
	WidgetSwitcher->SetActiveWidgetIndex(2);
}
