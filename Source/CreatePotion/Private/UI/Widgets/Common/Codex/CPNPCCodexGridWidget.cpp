// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPNPCCodexGridWidget.h"

#include "Components/UniformGridPanel.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "UI/Widgets/Common/Codex/CPNPCSlotWidget.h"

UCPNPCCodexGridWidget::UCPNPCCodexGridWidget(): GridColumn(3), GridRow(3)
{
}

void UCPNPCCodexGridWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	UGameInstance* GI = GetGameInstance();
	CodexSubsystem = GI ? GI->GetSubsystem<UCPCodexSubsystem>() : nullptr;
	if (!CodexSubsystem) return;
	
	GenerateSlots();
	SetNPCData();
}

void UCPNPCCodexGridWidget::BindEvents()
{
	Super::BindEvents();

	if (CodexSubsystem){
		CodexSubsystem->OnNPCCodexUpdated.AddUniqueDynamic(this, &UCPNPCCodexGridWidget::SetNPCData);
	}
}

void UCPNPCCodexGridWidget::UnbindEvents()
{
	if (CodexSubsystem){
		CodexSubsystem->OnNPCCodexUpdated.RemoveDynamic(this, &UCPNPCCodexGridWidget::SetNPCData);
	}

	Super::UnbindEvents();
}

void UCPNPCCodexGridWidget::SetNPCData()
{
	if (!CodexSubsystem) return;

	TArray<UCPNPCDataAsset*> NPCDatas = CodexSubsystem->GetRegisteredNPCDatas();

	// Slot에 NPCDA 할당
	for (int32 i = 0; i < SlotWidgets.Num(); ++i){
		if (NPCDatas.IsValidIndex(i)){
			SlotWidgets[i]->SetNPCData(NPCDatas[i]);
		}
	}
}

void UCPNPCCodexGridWidget::GenerateSlots()
{
	if (!IsValid(Grid_NPCCodex) || !IsValid(NPCSlotWidgetClass)) return;
	
	Grid_NPCCodex->ClearChildren();
	SlotWidgets.Empty();
	
	const int32 TotalSlotCount = GridRow * GridColumn;
	
	for (int32 i = 0; i < TotalSlotCount; i++){
		UCPNPCSlotWidget* NewSlot = CreateWidget<UCPNPCSlotWidget>(this, NPCSlotWidgetClass);
		if (!IsValid(NewSlot)) continue;
		
		const int32 Row = i / GridColumn;
		const int32 Col = i % GridColumn;
		
		Grid_NPCCodex->AddChildToUniformGrid(NewSlot, Row, Col);
		SlotWidgets.Add(NewSlot);
	}
}
