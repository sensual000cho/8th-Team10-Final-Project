// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPCodexItemSlotWidget.h"
// --- Data ---
#include "Data/CPForageableItemData.h"
// --- UI Components ---
#include "Components/Image.h"
#include "Components/Button.h"

void UCPCodexItemSlotWidget::BindEvents()
{
	Super::BindEvents();
	Button_Item->OnClicked.AddDynamic(this, &UCPCodexItemSlotWidget::HandleButtonClicked);
	
	InitSlotWidget();
}

void UCPCodexItemSlotWidget::UnbindEvents()
{
	Button_Item->OnClicked.RemoveDynamic(this, &UCPCodexItemSlotWidget::HandleButtonClicked);
	Super::UnbindEvents();
}

void UCPCodexItemSlotWidget::SetItemData(UCPForageableItemData* NewItemData)
{
	ItemData = NewItemData;
	InitSlotWidget();
}

void UCPCodexItemSlotWidget::HandleButtonClicked()
{
	if(!IsValid(ItemData))
	{
		UE_LOG(LogTemp, Error, TEXT("[CPCodexItemSlotWidget] 아이템 데이터가 지정되지 않았습니다."));
		return;
	}
	
	OnCodexItemSlotClicked.Broadcast(ItemData);
}

void UCPCodexItemSlotWidget::InitSlotWidget()
{
	if (!IsValid(ItemData)) return;
	
	UTexture2D* LoadedTexture = ItemData->CodexImage.LoadSynchronous();
	if (LoadedTexture)
	{
		Image_Item->SetBrushFromTexture(LoadedTexture);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[CPCodexItemSlotWidget] 아이템 아이콘 텍스쳐가 nullptr입니다."));
	}
	
}
