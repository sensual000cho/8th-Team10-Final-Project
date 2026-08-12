// 재료 정보 카드의 공용 표시와 Prop 변경 감지를 구현한다.

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
	// 스냅샷 표시로 전환할 때 이전 Actor의 변경 이벤트가 섞이지 않도록 관찰을 먼저 끝낸다.
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
			// 같은 위젯이 재구성돼도 동일한 델리게이트가 중복 등록되지 않게 AddUnique를 사용한다.
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
		// 재료가 사라지면 이전 재료의 예상 효과도 함께 지워 잔상이 남지 않게 한다.
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
	
	// 재료의 현재 효과를 기반으로 Tag 구성
	for (const TPair<FGameplayTag, int32>& Effect : Ingredient.CurrentEffects){
		if (!Effect.Key.IsValid()) continue;
		const int32* PreviewValue = PreviewEffects.Find(Effect.Key);
		
		UCPLabIngredientEffectRowWidget* EffectRow = 
			CreateWidget<UCPLabIngredientEffectRowWidget>(GetOwningPlayer(), EffectRowWidgetClass);
		if (!EffectRow) continue;
		
		EffectRow->SetEffectData(
			GetEffectDisplayName(Effect.Key), 
			Effect.Value, 
			PreviewValue != nullptr, 
			PreviewValue ? *PreviewValue : Effect.Value);
		
		VerticalBox_EffectRows->AddChildToVerticalBox(EffectRow);
	}
}

FText UCPLabIngredientInfoWidget::GetEffectDisplayName(const FGameplayTag& EffectTag) const
{
	// WBP별 한글 표시 이름이 지정돼 있으면 GameplayTag 문자열보다 우선한다.
	if (const FText* DisplayName = EffectDisplayNames.Find(EffectTag))
	{
		if (!DisplayName->IsEmpty())
		{
			return *DisplayName;
		}
	}

	FString FallbackName = EffectTag.ToString();
	int32 LastSeparatorIndex = INDEX_NONE;
	// 별도 표시 이름이 없으면 "Alchemy.BodyHeat"에서 마지막 조각인 "BodyHeat"만 보여준다.
	if (FallbackName.FindLastChar(TEXT('.'), LastSeparatorIndex))
	{
		FallbackName.RightChopInline(LastSeparatorIndex + 1);
	}

	return FText::FromString(FallbackName);
}
