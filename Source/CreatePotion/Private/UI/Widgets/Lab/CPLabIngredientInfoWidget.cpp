// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/Lab/CPLabIngredientInfoWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Data/CPForageableItemData.h"
#include "Engine/Texture2D.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "UI/Widgets/Lab/CPLabIngredientEffectRowWidget.h"

void UCPLabIngredientInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshWidget();
}

void UCPLabIngredientInfoWidget::SetIngredientInfo(const FCPLabIngredientInstance& InIngredient)
{
	UnbindObservedIngredient();
	ApplyIngredientInfo(InIngredient);
}

void UCPLabIngredientInfoWidget::SetObservedIngredient(ACPAlchemyProp* InIngredientProp)
{
	if (ObservedIngredientProp.Get() != InIngredientProp)
	{
		UnbindObservedIngredient();

		if (IsValid(InIngredientProp))
		{
			ObservedIngredientProp = InIngredientProp;
			InIngredientProp->OnAlchemyPropChanged.AddUniqueDynamic(
				this,
				&UCPLabIngredientInfoWidget::HandleObservedIngredientChanged);
		}
	}
	RefreshObservedIngredient();
}

void UCPLabIngredientInfoWidget::ClearObservedIngredient()
{
	UnbindObservedIngredient();
	ApplyIngredientInfo(FCPLabIngredientInstance{});
}

void UCPLabIngredientInfoWidget::SetHeaderText(const FText& InHeaderText)
{
	if (Text_Context)
	{
		Text_Context->SetText(InHeaderText);
	}
}

void UCPLabIngredientInfoWidget::ApplyIngredientInfo(const FCPLabIngredientInstance& InIngredient)
{
	if (!InIngredient.IsValid())
	{
		Ingredient = FCPLabIngredientInstance{};
		PreviewEffects.Reset();
		RefreshEmptyState();
		return;
	}

	Ingredient = InIngredient;

	RefreshWidget();
}

void UCPLabIngredientInfoWidget::RefreshObservedIngredient()
{
	ACPAlchemyProp* IngredientProp = ObservedIngredientProp.Get();
	if (!IsValid(IngredientProp))
	{
		ObservedIngredientProp.Reset();
		ApplyIngredientInfo(FCPLabIngredientInstance{});
		return;
	}

	ApplyIngredientInfo(IngredientProp->GetWorkingIngredient());
}

void UCPLabIngredientInfoWidget::UnbindObservedIngredient()
{
	if (ACPAlchemyProp* IngredientProp = ObservedIngredientProp.Get(); IsValid(IngredientProp))
	{
		IngredientProp->OnAlchemyPropChanged.RemoveDynamic(
			this,
			&UCPLabIngredientInfoWidget::HandleObservedIngredientChanged);
	}

	ObservedIngredientProp.Reset();
}

void UCPLabIngredientInfoWidget::HandleObservedIngredientChanged()
{
	RefreshObservedIngredient();
}

void UCPLabIngredientInfoWidget::UnbindEvents()
{
	UnbindObservedIngredient();
	Super::UnbindEvents();
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
	ClearObservedIngredient();
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
