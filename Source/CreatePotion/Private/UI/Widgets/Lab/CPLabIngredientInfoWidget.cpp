// 재료 정보 카드의 공용 표시와 Prop 변경 감지를 구현한다.

#include "UI/Widgets/Lab/CPLabIngredientInfoWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Data/CPForageableItemData.h"
#include "Data/CPTagDefinitionTypes.h"
#include "Engine/Texture2D.h"
#include "Lab/Actor/CPThrowablePropBase.h"
#include "Settings/CPDTSettings.h"
#include "UI/Widgets/Lab/CPLabIngredientEffectRowWidget.h"

void UCPLabIngredientInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshWidget();
}

void UCPLabIngredientInfoWidget::SetIngredientProp(ACPThrowablePropBase* InProp)
{
	if (ObservedIngredientProp.Get() != InProp)
	{
		UnbindObservedIngredient();

		if (IsValid(InProp))
		{
			ObservedIngredientProp = InProp;
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
		RefreshEmptyState();
		return;
	}

	Ingredient = InIngredient;

	RefreshWidget();
}

void UCPLabIngredientInfoWidget::RefreshObservedIngredient()
{
	ACPThrowablePropBase* IngredientProp = ObservedIngredientProp.Get();
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
	ObservedIngredientProp.Reset();
}

void UCPLabIngredientInfoWidget::UnbindEvents()
{
	UnbindObservedIngredient();
	Super::UnbindEvents();
}

void UCPLabIngredientInfoWidget::ClearPreviewEffects()
{
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
		// 유효하지 않은 원본 DataAsset은 정상 재료로 표시하지 않고 빈 상태로 되돌린다.
		ClearIngredientInfo();
		return;
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
}

void UCPLabIngredientInfoWidget::RebuildEffectRows()
{
	if (!VerticalBox_EffectRows) return;

	VerticalBox_EffectRows->ClearChildren();

	const UCPForageableItemData* ItemData = Ingredient.SourceItemData.Get();
	if (!ItemData || !EffectRowWidgetClass) return;
	
	const TArray<FGameplayTag>& DisplayEffectTags = 
		Ingredient.CurrentEffects.IsEmpty() ? ItemData->TagAxes : Ingredient.CurrentEffects;
	
	// 재료의 현재 효과를 기반으로 Tag 구성
	for (const FGameplayTag& EffectTag : DisplayEffectTags){
		if (!EffectTag.IsValid()) continue;
		
		UCPLabIngredientEffectRowWidget* EffectRow = 
			CreateWidget<UCPLabIngredientEffectRowWidget>(GetOwningPlayer(), EffectRowWidgetClass);
		if (!EffectRow) continue;
		
		EffectRow->SetEffectData(GetEffectDisplayName(EffectTag));
		
		VerticalBox_EffectRows->AddChildToVerticalBox(EffectRow);
	}
}

FText UCPLabIngredientInfoWidget::GetEffectDisplayName(const FGameplayTag& EffectTag) const
{
	if (!EffectTag.IsValid()) return FText::GetEmpty();
	
	const UCPDTSettings* DTSettings = GetDefault<UCPDTSettings>();
	UDataTable* TagDefinitionTable = DTSettings ? DTSettings->TagDefinitionTable.LoadSynchronous() : nullptr;
	if (!TagDefinitionTable) return FText::GetEmpty();
	
	TArray<FCPTagDefinitionRow*> Rows;
	TagDefinitionTable->GetAllRows<FCPTagDefinitionRow>(TEXT("EffectTagDefinition"), Rows);
	
	for (const FCPTagDefinitionRow* Row : Rows){
		if (Row && Row->Tag == EffectTag) {
			return Row->DisplayName;
		}
	}
	
	return FText::GetEmpty();
}
