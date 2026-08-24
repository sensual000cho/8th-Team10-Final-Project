// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPCodexItemGridWidget.h"
#include "UI/Widgets/Common/Codex/CPCodexItemSlotWidget.h"
#include "Components/UniformGridPanel.h"

UCPCodexItemGridWidget::UCPCodexItemGridWidget()
	: GridColumn(3)
	, GridRow(3)
{
}

void UCPCodexItemGridWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
	if (!CodexSubsystem) return;
	
	GenerateSlots();
}

void UCPCodexItemGridWidget::BindEvents()
{
	Super::BindEvents();
	CodexSubsystem->OnForageableCodexUpdated.AddUniqueDynamic(this, &UCPCodexItemGridWidget::SetCodexEntries);
}

void UCPCodexItemGridWidget::UnbindEvents()
{
	Super::UnbindEvents();
	CodexSubsystem->OnForageableCodexUpdated.RemoveDynamic(this, &UCPCodexItemGridWidget::SetCodexEntries);
}

void UCPCodexItemGridWidget::HandleSlotClicked(const FCPForageableCodexEntry& SelectedCodexEntry)
{
	OnCodexGridItemClicked.Broadcast(SelectedCodexEntry);
}

void UCPCodexItemGridWidget::SetCodexEntries()
{
	
	TArray<FCPForageableCodexEntry> CodexEntries = CodexSubsystem->GetForageableCodexEntries();
	
	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		if (CodexEntries.IsValidIndex(i))
		{
			SlotWidgets[i]->SetCodexEntry(CodexEntries[i]);
		}
	}
}

void UCPCodexItemGridWidget::GenerateSlots()
{
	if (!IsValid(Grid_ItemCodex) || !IsValid(CodexItemSlotWidgetClass))
	{
		return;
	}
	
	Grid_ItemCodex->ClearChildren();
	SlotWidgets.Empty();
	
	const int32 TotalSlotCount = GridRow * GridColumn;
	
	for (int32 i = 0; i < TotalSlotCount; i++)
	{
		UCPCodexItemSlotWidget* NewSlot = CreateWidget<UCPCodexItemSlotWidget>(this, CodexItemSlotWidgetClass);
		if (!IsValid(NewSlot)) continue;
		
		const int32 Row = i / GridColumn;
		const int32 Col = i % GridColumn;
		
		Grid_ItemCodex->AddChildToUniformGrid(NewSlot, Row, Col);
		
		NewSlot->OnCodexItemSlotClicked.AddDynamic(this, &UCPCodexItemGridWidget::HandleSlotClicked);
		
		SlotWidgets.Add(NewSlot);
	}
}
