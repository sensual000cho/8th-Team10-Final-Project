#include "UI/Widgets/Lab/CPLabCarriedIngredientWidget.h"

#include "Character/CPCarryComponent.h"
#include "Components/TextBlock.h"
#include "Data/CPForageableItemData.h"
#include "GameFramework/Pawn.h"
#include "Lab/Actor/CPThrowablePropBase.h"

void UCPLabCarriedIngredientWidget::NativeConstruct()
{
	// Super 호출 중 UCPBaseUserWidget이 가상 BindEvents를 호출하므로 세션 바인딩은 이미 끝난 상태다.
	Super::NativeConstruct();

	SetHeaderText(FText::FromString(TEXT("현재 운반중인 물체")));
	SetVisibility(ESlateVisibility::Collapsed);
	HandlePropChanged(IsValid(BoundCarryComponent) ? BoundCarryComponent->GetHeldProp() : nullptr);
}

void UCPLabCarriedIngredientWidget::BindEvents()
{
	Super::BindEvents();

	if (APawn* OwningPawn = GetOwningPlayerPawn())
	{
		BoundCarryComponent = OwningPawn->FindComponentByClass<UCPCarryComponent>();
		if (IsValid(BoundCarryComponent))
		{
			BoundCarryComponent->OnHeldPropChanged.AddUniqueDynamic(
				this, &UCPLabCarriedIngredientWidget::HandlePropChanged);
		}
	}
}

void UCPLabCarriedIngredientWidget::UnbindEvents()
{
	if (IsValid(BoundCarryComponent))
	{
		BoundCarryComponent->OnHeldPropChanged.RemoveDynamic(
			this,
			&UCPLabCarriedIngredientWidget::HandlePropChanged);
	}

	BoundCarryComponent = nullptr;

	Super::UnbindEvents();
}

void UCPLabCarriedIngredientWidget::HandlePropChanged(ACPThrowablePropBase* Prop)
{
	if (IsValid(Prop))
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);
		SetIngredientProp(Prop);
		
		Txt_TagEffect->SetVisibility(ESlateVisibility::Collapsed);
		
		// 재료의 효과 태그를 가져옴
		const FCPItemInstance& WorkingIngredient = Prop->GetWorkingIngredient();
		if (!WorkingIngredient.CurrentEffects.IsEmpty()) return;
		
		const UCPForageableItemData* ItemData = WorkingIngredient.SourceItemData.Get();
		if (!ItemData) return;
		
		if (!ItemData->TagAxes.IsEmpty()){
			Txt_TagEffect->SetText(GetEffectDisplayName(ItemData->TagAxes[0]));
			Txt_TagEffect->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		return;
	}

	ClearObservedIngredient();
	SetVisibility(ESlateVisibility::Collapsed);
}