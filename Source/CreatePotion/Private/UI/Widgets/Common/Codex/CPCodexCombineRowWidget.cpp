// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPCodexCombineRowWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Data/CPForageableItemData.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"

void UCPCodexCombineRowWidget::SetCombinationData(const FCPTagCodexDisplayData& LeftData,
	const FCPTagCodexDisplayData& RightData, const FCPTagCodexDisplayData& ResultData, bool bShowImages)
{
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	UCPCodexSubsystem* CodexSubsystem = GameInstance ? GameInstance->GetSubsystem<UCPCodexSubsystem>() : nullptr;
	
	if (bShowImages){
		SizeBox->SetHeightOverride(SizeBox->GetHeightOverride());
		
		UCPForageableItemData* LeftItemData = CodexSubsystem ? CodexSubsystem->FindRegisteredItemByTag(LeftData.Tag) : nullptr;
		UCPForageableItemData* RightItemData = CodexSubsystem ? CodexSubsystem->FindRegisteredItemByTag(RightData.Tag) : nullptr;
	
		ApplyIngredientImage(Img_LeftImage, LeftItemData);
		ApplyIngredientImage(Img_RightImage, RightItemData);	
	}else{
		SizeBox->ClearHeightOverride();
		Img_LeftImage->SetVisibility(ESlateVisibility::Collapsed);
		Img_RightImage->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	const bool bLeftTagDiscovered = CodexSubsystem ? CodexSubsystem->IsTagDiscovered(LeftData.Tag) : false;
	const bool bRightTagDiscovered = CodexSubsystem ? CodexSubsystem->IsTagDiscovered(RightData.Tag) : false;
	const bool bResultTagDiscovered = CodexSubsystem ? CodexSubsystem->IsTagDiscovered(ResultData.Tag) : false;
	
	ApplyDisplay(Border_Left, Txt_LeftTag, LeftData, bLeftTagDiscovered);
	ApplyDisplay(Border_Right, Txt_RightTag, RightData, bRightTagDiscovered);
	ApplyDisplay(Border_Result, Txt_ResultTag, ResultData, bResultTagDiscovered);
}

void UCPCodexCombineRowWidget::ApplyIngredientImage(UImage* ImageWidget, UCPForageableItemData* ItemData)
{
	if (!ImageWidget) return;
	
	UTexture2D* Texture = ItemData ? ItemData->CodexImage.LoadSynchronous() : nullptr;
	if (!Texture){
		ImageWidget->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	ImageWidget->SetBrushFromTexture(Texture);
	ImageWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCPCodexCombineRowWidget::ApplyDisplay(UBorder* Border, UTextBlock* TextBlock, const FCPTagCodexDisplayData& Data,
	bool bDiscovered)
{
	const bool bShowTagDefinition = bDiscovered && Data.Color.IsSet();
	
	TextBlock->SetText(bShowTagDefinition ? Data.Text : FText::FromString(TEXT("???")));
	
	FLinearColor BorderColor = UnDiscoveredTagColor;
	if (bShowTagDefinition){
		BorderColor = Data.Color.GetValue();
		BorderColor.A *= TagColorOpacity;
	}
	
	Border->SetBrushColor(BorderColor);
}
