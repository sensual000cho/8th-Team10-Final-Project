// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Lab/CPLabPotionResultWidget.h"

#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "UI/Widgets/Lab/CPLabIngredientEffectRowWidget.h"

void UCPLabPotionResultWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCPLabPotionResultWidget::ReBuildEffectRows(const TArray<FGameplayTag>& EffectTotals)
{
	if (!VB_EffectRows) return;
	
	VB_EffectRows->ClearChildren();
	
	if (EffectTotals.IsEmpty() || !EffectRowWidgetClass){
		if (Txt_NoResult){
			Txt_NoResult->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		return;
	}
	
	if (Txt_NoResult){
		Txt_NoResult->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	for (const FGameplayTag& EffectTag : EffectTotals){
		if (!EffectTag.IsValid()) continue;
		
		UCPLabIngredientEffectRowWidget* EffectRow = 
			CreateWidget<UCPLabIngredientEffectRowWidget>(GetOwningPlayer(), EffectRowWidgetClass);
		if (!EffectRow) continue;
		
		FString EffectName = EffectTag.ToString();
		int32 LastSeparatorIndex = INDEX_NONE;
		if (EffectName.FindLastChar(TEXT('.'), LastSeparatorIndex)){
			EffectName.RightChopInline(LastSeparatorIndex + 1);
		}
		
		EffectRow->SetEffectData(FText::FromString(EffectName));
		
		VB_EffectRows->AddChildToVerticalBox(EffectRow);
	}
	
	if (VB_EffectRows->GetChildrenCount() == 0 && Txt_NoResult){
		Txt_NoResult->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}
