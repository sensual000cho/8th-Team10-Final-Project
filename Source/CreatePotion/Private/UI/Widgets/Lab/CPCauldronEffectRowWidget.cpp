// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Lab/CPCauldronEffectRowWidget.h"

#include "Components/TextBlock.h"

void UCPCauldronEffectRowWidget::SetEffectData(const FText& EffectName, const FLinearColor& EffectColor)
{
	Txt_EffectName->SetText(EffectName);
	Txt_EffectName->SetColorAndOpacity(EffectColor);
}
