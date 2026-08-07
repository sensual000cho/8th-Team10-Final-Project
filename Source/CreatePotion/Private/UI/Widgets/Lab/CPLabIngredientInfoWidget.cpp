// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/Lab/CPLabIngredientInfoWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Data/CPForageableItemData.h"
#include "Engine/Texture2D.h"

void UCPLabIngredientInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Ingredient.IsValid())
	{
		RefreshWidget();
		ShowIngredientInfo();
	}
	else
	{
		// 테스트 용으로 숨기기 기능 없애둠
		//HideIngredientInfo();		
	}
}

void UCPLabIngredientInfoWidget::SetIngredientInfo(
	const FCPLabIngredientInstance& InIngredient,
	const FText& InContextText)
{
	if (!InIngredient.IsValid())
	{
		ClearIngredientInfo();
		return;
	}

	Ingredient = InIngredient;
	ContextText = InContextText;

	RefreshWidget();
	ShowIngredientInfo();
}

void UCPLabIngredientInfoWidget::SetPreviewEffects(
	const TMap<FGameplayTag, int32>& InPreviewEffects)
{
	PreviewEffects = InPreviewEffects;
	RefreshPreview();
}

void UCPLabIngredientInfoWidget::ClearPreviewEffects()
{
	PreviewEffects.Reset();
	RefreshPreview();
}

void UCPLabIngredientInfoWidget::ClearIngredientInfo()
{
	Ingredient = FCPLabIngredientInstance{};
	PreviewEffects.Reset();
	ContextText = FText::GetEmpty();

	HideIngredientInfo();
}

void UCPLabIngredientInfoWidget::ShowIngredientInfo()
{
	if (!Ingredient.IsValid())
	{
		return;
	}

	// 클릭 안되는 말 그대로 "표시용" UI
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCPLabIngredientInfoWidget::HideIngredientInfo()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

bool UCPLabIngredientInfoWidget::HasIngredientInfo() const
{
	return Ingredient.IsValid();
}

void UCPLabIngredientInfoWidget::RefreshWidget()
{
	if (!Ingredient.IsValid())
	{
		HideIngredientInfo();
		return;
	}

	const UCPForageableItemData* ItemData = Ingredient.SourceItemData.Get();
	if (!ItemData)
	{
		ClearIngredientInfo();
		return;
	}

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

	if (Text_EffectSummary)
	{
		Text_EffectSummary->SetText(BuildEffectSummary());
	}

	RefreshPreview();
}

void UCPLabIngredientInfoWidget::RefreshPreview()
{
	const FText PreviewSummary = BuildPreviewDeltaSummary();
	const bool bShouldShowPreview = !PreviewSummary.IsEmpty();

	if (VerticalBox_DeltaArea)
	{
		VerticalBox_DeltaArea->SetVisibility(
			bShouldShowPreview
				? ESlateVisibility::HitTestInvisible
				: ESlateVisibility::Collapsed);
	}

	if (Text_PreviewDeltaSummary)
	{
		Text_PreviewDeltaSummary->SetText(PreviewSummary);
	}
}

FText UCPLabIngredientInfoWidget::BuildEffectSummary() const
{
	if (!Ingredient.IsValid())
	{
		return FText::GetEmpty();
	}

	const UCPForageableItemData* ItemData = Ingredient.SourceItemData.Get();
	if (!ItemData)
	{
		return FText::GetEmpty();
	}

	TArray<FString> EffectLines;
	EffectLines.Reserve(ItemData->TagAxes.Num());

	// TMap 순회 순서는 고정되지 않으므로 DataAsset의 축 순서를 표시 순서로 사용한다.
	for (const FAlchemyProperty& Property : ItemData->TagAxes)
	{
		if (!Property.Tag.IsValid())
		{
			continue;
		}

		const int32 CurrentValue = Ingredient.CurrentEffects.FindRef(Property.Tag);
		EffectLines.Add(FString::Printf(
			TEXT("%s: %d"),
			*Property.Tag.ToString(),
			CurrentValue));
	}

	return EffectLines.IsEmpty()
		? FText::GetEmpty()
		: FText::FromString(FString::Join(EffectLines, TEXT("\n")));
}

FText UCPLabIngredientInfoWidget::BuildPreviewDeltaSummary() const
{
	if (!Ingredient.IsValid() || PreviewEffects.IsEmpty())
	{
		return FText::GetEmpty();
	}

	const UCPForageableItemData* ItemData = Ingredient.SourceItemData.Get();
	if (!ItemData)
	{
		return FText::GetEmpty();
	}

	TArray<FString> DeltaLines;
	DeltaLines.Reserve(ItemData->TagAxes.Num());

	for (const FAlchemyProperty& Property : ItemData->TagAxes)
	{
		if (!Property.Tag.IsValid())
		{
			continue;
		}

		const int32* PreviewValue = PreviewEffects.Find(Property.Tag);
		if (!PreviewValue)
		{
			continue;
		}

		const int32 CurrentValue = Ingredient.CurrentEffects.FindRef(Property.Tag);
		if (CurrentValue == *PreviewValue)
		{
			continue;
		}

		DeltaLines.Add(FString::Printf(
			TEXT("%s: %d -> %d"),
			*Property.Tag.ToString(),
			CurrentValue,
			*PreviewValue));
	}

	return DeltaLines.IsEmpty()
		? FText::GetEmpty()
		: FText::FromString(FString::Join(DeltaLines, TEXT("\n")));
}
