#include "UI/Widgets/Lab/CPLabCarriedIngredientWidget.h"

#include "Character/CPCarryComponent.h"
#include "Character/CPInteractionComponent.h"
#include "GameFramework/Pawn.h"
#include "Lab/Actor/CPThrowablePropBase.h"

void UCPLabCarriedIngredientWidget::NativeConstruct()
{
	// Super 호출 중 UCPBaseUserWidget이 가상 BindEvents를 호출하므로 세션 바인딩은 이미 끝난 상태다.
	Super::NativeConstruct();

	SetHeaderText(FText::FromString(TEXT("현재 운반중인 물체")));
	SetVisibility(ESlateVisibility::HitTestInvisible);
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
	if (BoundInteractionComponent.IsValid())
	{
		BoundInteractionComponent->OnPromptChanged.RemoveDynamic(
			this,
			&UCPLabCarriedIngredientWidget::HandleInteractionFocusChanged);
	}

	BoundInteractionComponent.Reset();
	HeldProp.Reset();

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
	// 슬롯 호버 카드와 달리 운반 카드는 빈손이어도 "비어 있음" 상태로 계속 표시한다.
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (IsValid(Prop))
	{
		SetIngredientProp(Prop);
		BindPreviewProp(Prop);
		ClearPreviewEffects();
		return;
	}

	HeldProp.Reset();
	ClearObservedIngredient();
	ClearPreviewEffects();
}

void UCPLabCarriedIngredientWidget::HandleInteractionFocusChanged(FText Prompt, FName TargetName, ECPInteractionDisplayState DisplayState)
{
	(void)Prompt;
	(void)TargetName;

	AActor* TargetActor = BoundInteractionComponent.IsValid()
		? BoundInteractionComponent->GetCurrentTarget()
		: nullptr;

	if (IsValid(TargetActor))
	{
		ClearPreviewEffects();
	}
}

void UCPLabCarriedIngredientWidget::HandlePreviewIngredientChanged()
{
	ClearPreviewEffects();
}

void UCPLabCarriedIngredientWidget::BindPreviewProp(ACPThrowablePropBase* Prop)
{
	if (HeldProp.Get() == Prop) return;

	HeldProp.Reset();
	if (!IsValid(Prop)) return;

	HeldProp = Prop;
}