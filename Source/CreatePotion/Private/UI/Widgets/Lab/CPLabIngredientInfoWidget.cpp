// 재료 정보 카드의 공용 표시와 Prop 변경 감지를 구현한다.

#include "UI/Widgets/Lab/CPLabIngredientInfoWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/CPForageableItemData.h"
#include "Data/CPTagDefinitionTypes.h"
#include "Engine/Texture2D.h"
#include "Lab/Actor/CPThrowablePropBase.h"
#include "Settings/CPDTSettings.h"

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
	ApplyIngredientInfo(FCPItemInstance{});
}

void UCPLabIngredientInfoWidget::SetHeaderText(const FText& InHeaderText)
{
	if (Text_Context)
	{
		Text_Context->SetText(InHeaderText);
	}
}

void UCPLabIngredientInfoWidget::ApplyIngredientInfo(const FCPItemInstance& InIngredient)
{
	if (!InIngredient.IsValid())
	{
		Ingredient = FCPItemInstance{};
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
		ApplyIngredientInfo(FCPItemInstance{});
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
		FText MaterialName = ItemData->DisplayName;
		
		// CurrentEffects가 존재하면 포션으로 판단
		if (!Ingredient.CurrentEffects.IsEmpty()){
			TArray<FText> PotionTagNames;
			PotionTagNames.Reserve(Ingredient.CurrentEffects.Num()+1);
			
			// 저장된 태그 순서 사용
			for (const FGameplayTag& EffectTag : Ingredient.CurrentEffects){
				if (!EffectTag.IsValid()) continue;
				PotionTagNames.Add(GetEffectDisplayName(EffectTag));
			}
			PotionTagNames.Add(ItemData->DisplayName);
			
			MaterialName = FText::Join(FText::FromString(TEXT(" ")), PotionTagNames);
		}
		// 마지막에는 DA의 이름 사용
		Text_MaterialName->SetText(MaterialName);
	}

	if (Image_MaterialIcon)
	{
		UTexture2D* IconTexture = ItemData->Icon.LoadSynchronous();
		Image_MaterialIcon->SetBrushFromTexture(IconTexture);
		Image_MaterialIcon->SetVisibility(
			IconTexture ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
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
