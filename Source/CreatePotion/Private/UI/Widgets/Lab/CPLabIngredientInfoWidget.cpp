// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/Lab/CPLabIngredientInfoWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Data/CPForageableItemData.h"
#include "Engine/Texture2D.h"
#include "UI/Widgets/Lab/CPLabIngredientEffectRowWidget.h"

void UCPLabIngredientInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshWidget();
}

void UCPLabIngredientInfoWidget::SetIngredientInfo(
	const FCPLabIngredientInstance& InIngredient,
	const FText& InContextText)
{
	ContextText = InContextText;

	if (!InIngredient.IsValid())
	{
		ClearIngredientInfo();
		return;
	}

	Ingredient = InIngredient;

	RefreshWidget();
}

void UCPLabIngredientInfoWidget::SetPreviewEffects(
	const TMap<FGameplayTag, int32>& InPreviewEffects)
{
	PreviewEffects = InPreviewEffects;
	RebuildEffectRows();
}

void UCPLabIngredientInfoWidget::ClearPreviewEffects()
{
	PreviewEffects.Reset();
	RebuildEffectRows();
}

void UCPLabIngredientInfoWidget::ClearIngredientInfo()
{
	Ingredient = FCPLabIngredientInstance{};
	PreviewEffects.Reset();
	RefreshEmptyState();
}

bool UCPLabIngredientInfoWidget::HasIngredientInfo() const
{
	return Ingredient.IsValid();
}

void UCPLabIngredientInfoWidget::RefreshWidget()
{
	if (!Ingredient.IsValid())
	{
		RefreshEmptyState();
		return;
	}

	const UCPForageableItemData* ItemData = Ingredient.SourceItemData.Get();
	if (!ItemData)
	{
		ClearIngredientInfo();
		return;
	}

	// This fixed HUD card always remains present.
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (Text_Context)
	{
		Text_Context->SetText(ContextText);
	}

	if (Text_MaterialName)
	{
		Text_MaterialName->SetText(ItemData->DisplayName);
	}

	if (Image_MaterialIcon)
	{
		UTexture2D* IconTexture = ItemData->Icon.LoadSynchronous();
		Image_MaterialIcon->SetBrushFromTexture(IconTexture);
		Image_MaterialIcon->SetVisibility(
			IconTexture ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	RebuildEffectRows();
}

void UCPLabIngredientInfoWidget::RefreshEmptyState()
{
	// Preserve the designer text when no context has been supplied yet.
	if (Text_Context && !ContextText.IsEmpty())
	{
		Text_Context->SetText(ContextText);
	}

	if (Text_MaterialName)
	{
		Text_MaterialName->SetText(EmptyIngredientText);
	}

	if (Image_MaterialIcon)
	{
		Image_MaterialIcon->SetBrushFromTexture(nullptr);
		Image_MaterialIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (VerticalBox_EffectRows)
	{
		VerticalBox_EffectRows->ClearChildren();
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCPLabIngredientInfoWidget::RebuildEffectRows()
{
	if (!VerticalBox_EffectRows)
	{
		return;
	}

	VerticalBox_EffectRows->ClearChildren();

	const UCPForageableItemData* ItemData = Ingredient.SourceItemData.Get();
	if (!ItemData || !EffectRowWidgetClass)
	{
		return;
	}

	// Preserve the authored DataAsset order instead of relying on TMap iteration order.
	for (const FAlchemyProperty& Property : ItemData->TagAxes)
	{
		if (!Property.Tag.IsValid())
		{
			continue;
		}

		const int32 CurrentValue = Ingredient.CurrentEffects.FindRef(Property.Tag);
		const int32* PreviewValue = PreviewEffects.Find(Property.Tag);

		UCPLabIngredientEffectRowWidget* EffectRow =
			CreateWidget<UCPLabIngredientEffectRowWidget>(GetOwningPlayer(), EffectRowWidgetClass);
		if (!EffectRow)
		{
			continue;
		}

		EffectRow->SetEffectData(
			GetEffectDisplayName(Property.Tag),
			CurrentValue,
			PreviewValue != nullptr,
			PreviewValue ? *PreviewValue : CurrentValue);

		VerticalBox_EffectRows->AddChildToVerticalBox(EffectRow);
	}
}

FText UCPLabIngredientInfoWidget::GetEffectDisplayName(const FGameplayTag& EffectTag) const
{
	if (const FText* DisplayName = EffectDisplayNames.Find(EffectTag))
	{
		if (!DisplayName->IsEmpty())
		{
			return *DisplayName;
		}
	}

	FString FallbackName = EffectTag.ToString();
	int32 LastSeparatorIndex = INDEX_NONE;
	if (FallbackName.FindLastChar(TEXT('.'), LastSeparatorIndex))
	{
		FallbackName.RightChopInline(LastSeparatorIndex + 1);
	}

	return FText::FromString(FallbackName);
}
