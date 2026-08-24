// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPCodexItemSlotWidget.h"
// --- Data ---
#include "Data/CPForageableItemData.h"
// --- UI Components ---
#include "Components/Image.h"
#include "Components/Button.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"

void UCPCodexItemSlotWidget::BindEvents()
{
	Super::BindEvents();
	Button_Item->OnClicked.AddDynamic(this, &UCPCodexItemSlotWidget::HandleSlotClicked);
	
	RefreshImage();
}

void UCPCodexItemSlotWidget::UnbindEvents()
{
	Button_Item->OnClicked.RemoveDynamic(this, &UCPCodexItemSlotWidget::HandleSlotClicked);
	Super::UnbindEvents();
}

void UCPCodexItemSlotWidget::SetItemData(UCPForageableItemData* NewItemData)
{
	ItemData = NewItemData;
	RefreshImage();
}

void UCPCodexItemSlotWidget::RefreshImage()
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


void UCPCodexItemSlotWidget::HandleSlotClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("[CodexItemSlotWidget] HandleSlotClicked 호출됨"));
	if(!IsValid(ItemData))
	{
		UE_LOG(LogTemp, Error, TEXT("[CPCodexItemSlotWidget] 아이템 데이터가 유효하지 않습니다."));
		return;
	}
	
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
	if (!CodexSubsystem) return;
	
	CodexSubsystem->OnCodexItemSelected.Broadcast(ItemData);
}

