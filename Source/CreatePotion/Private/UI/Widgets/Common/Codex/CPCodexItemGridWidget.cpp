// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPCodexItemGridWidget.h"
#include "UI/Widgets/Common/Codex/CPCodexItemSlotWidget.h"
#include "Components/UniformGridPanel.h"

UCPCodexItemGridWidget::UCPCodexItemGridWidget()
	: GridColumn(3)
	, GridRow(3)
{
}

void UCPCodexItemGridWidget::HandleSlotClicked(UCPForageableItemData* ItemData)
{
	OnCodexGridItemClicked.Broadcast(ItemData);
}

void UCPCodexItemGridWidget::InitGrid(const TArray<class UCPForageableItemData*>& ItemDatas)
{
	GenerateSlots();
	
	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		if (ItemDatas.IsValidIndex(i))
		{
			SlotWidgets[i]->SetItemData(ItemDatas[i]);
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
