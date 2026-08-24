// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPCodexItemSlotWidget.h"
// --- Data ---
#include "Data/CPForageableItemData.h"
// --- UI Components ---
#include "Components/Image.h"
#include "Components/Button.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"

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

void UCPCodexItemSlotWidget::SetCodexEntry(const FCPForageableCodexEntry& NewCodexEntry)
{
	CodexEntry.Entry = NewCodexEntry.Entry;
	CodexEntry.Level = NewCodexEntry.Level;
	RefreshImage();
}

void UCPCodexItemSlotWidget::RefreshImage()
{
	if (!IsValid(CodexEntry.Entry)) return;
	
	UTexture2D* LoadedTexture = CodexEntry.Entry->CodexImage.LoadSynchronous();
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
	if(!IsValid(CodexEntry.Entry))
	{
		UE_LOG(LogTemp, Error, TEXT("[CPCodexItemSlotWidget] 아이템 데이터가 유효하지 않습니다."));
		return;
	}
	
	OnCodexItemSlotClicked.Broadcast(CodexEntry);
}

