// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/Lab/CPLabIngredientEffectRowWidget.h"

#include "Components/TextBlock.h"
#include "Components/Widget.h"

#define LOCTEXT_NAMESPACE "CPLabIngredientEffectRowWidget"

void UCPLabIngredientEffectRowWidget::SetEffectData(
	const FText& InEffectName,
	int32 InCurrentLevel,
	bool bInHasPreview,
	int32 InPreviewLevel)
{
	if (Text_EffectName)
	{
		Text_EffectName->SetText(InEffectName);
	}

	if (Text_CurrentLevel)
	{
		Text_CurrentLevel->SetText(FormatLevel(InCurrentLevel));
	}

	if (!bInHasPreview)
	{
		DeltaState = ECPLabIngredientEffectDeltaState::None;

		if (PreviewGroup)
		{
			PreviewGroup->SetVisibility(ESlateVisibility::Collapsed);
		}

		BP_ApplyDeltaStyle(DeltaState);
		return;
	}

	if (InPreviewLevel > InCurrentLevel)
	{
		DeltaState = ECPLabIngredientEffectDeltaState::Increased;
	}
	else if (InPreviewLevel < InCurrentLevel)
	{
		DeltaState = ECPLabIngredientEffectDeltaState::Decreased;
	}
	else
	{
		DeltaState = ECPLabIngredientEffectDeltaState::Unchanged;
	}

	if (Text_PreviewLevel)
	{
		Text_PreviewLevel->SetText(FormatLevel(InPreviewLevel));
	}

	if (PreviewGroup)
	{
		PreviewGroup->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	BP_ApplyDeltaStyle(DeltaState);
}

ECPLabIngredientEffectDeltaState UCPLabIngredientEffectRowWidget::GetDeltaState() const
{
	return DeltaState;
}

FText UCPLabIngredientEffectRowWidget::FormatLevel(int32 Level)
{
	return FText::Format(
		LOCTEXT("EffectLevelFormat", "Lv. {0}"),
		FText::AsNumber(Level));
}

#undef LOCTEXT_NAMESPACE
