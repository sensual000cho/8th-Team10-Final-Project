#include "UI/Widgets/Lab/CPLabCarriedIngredientWidget.h"

#include "Character/CPCarryComponent.h"
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
		return;
	}

	ClearObservedIngredient();
	SetVisibility(ESlateVisibility::Collapsed);
}