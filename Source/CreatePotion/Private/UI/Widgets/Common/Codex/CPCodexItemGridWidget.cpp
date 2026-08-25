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
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
	if (!CodexSubsystem) return;
	
	Super::NativeConstruct();
	
	GenerateSlots();
	SetItemData();
}

void UCPCodexItemGridWidget::BindEvents()
{
	Super::BindEvents();
	CodexSubsystem->OnForageableCodexUpdated.AddUniqueDynamic(this, &UCPCodexItemGridWidget::SetItemData);
}

void UCPCodexItemGridWidget::UnbindEvents()
{
	Super::UnbindEvents();
	CodexSubsystem->OnForageableCodexUpdated.RemoveDynamic(this, &UCPCodexItemGridWidget::SetItemData);
}

void UCPCodexItemGridWidget::SetItemData()
{
	TArray<UCPForageableItemData*> ItemDatas = CodexSubsystem->GetRegisteredForageableItems();
	
	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		if (ItemDatas.IsValidIndex(i))
		{
			UE_LOG(LogTemp, Warning, TEXT("Slot [%d]에 할당된 아이템 이름: %s"), i, *ItemDatas[i]->GetName());
			SlotWidgets[i]->SetItemData(ItemDatas[i]);
		}
		else
		{
			continue;
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
		
		SlotWidgets.Add(NewSlot);
	}
}
