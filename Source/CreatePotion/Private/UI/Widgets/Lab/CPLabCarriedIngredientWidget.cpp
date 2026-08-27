#include "UI/Widgets/Lab/CPLabCarriedIngredientWidget.h"

#include "Character/CPCarryComponent.h"
#include "Character/CPInteractionComponent.h"
#include "GameFramework/Pawn.h"
#include "Lab/Actor/CPAlchemyProp.h"

void UCPLabCarriedIngredientWidget::NativeConstruct()
{
	// Super 호출 중 UCPBaseUserWidget이 가상 BindEvents를 호출하므로 세션 바인딩은 이미 끝난 상태다.
	Super::NativeConstruct();

	SetHeaderText(FText::FromString(TEXT("현재 운반중인 재료")));
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
	UnbindPreviewIngredient();

	if (IsValid(BoundCarryComponent))
	{
		BoundCarryComponent->OnHeldPropChanged.RemoveDynamic(
			this,
			&UCPLabCarriedIngredientWidget::HandlePropChanged);
	}

	BoundCarryComponent = nullptr;

	Super::UnbindEvents();
}

void UCPLabCarriedIngredientWidget::HandlePropChanged(ACPThrowablePropBase* HeldProp)
{
	// 슬롯 호버 카드와 달리 운반 카드는 빈손이어도 "비어 있음" 상태로 계속 표시한다.
	SetVisibility(ESlateVisibility::HitTestInvisible);

	ACPAlchemyProp* HeldIngredientProp = Cast<ACPAlchemyProp>(HeldProp);
	if (IsValid(HeldIngredientProp))
	{
		SetObservedIngredient(HeldIngredientProp);
		BindPreviewIngredient(HeldIngredientProp);
		ClearPreviewEffects();
		return;
	}

	UnbindPreviewIngredient();
	ClearObservedIngredient();
	ClearPreviewEffects();
}

void UCPLabCarriedIngredientWidget::HandleInteractionFocusChanged(FText Prompt, FName TargetName)
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

void UCPLabCarriedIngredientWidget::BindPreviewIngredient(ACPAlchemyProp* IngredientProp)
{
	if (PreviewIngredient.Get() == IngredientProp) return;

	UnbindPreviewIngredient();
	if (!IsValid(IngredientProp)) return;

	PreviewIngredient = IngredientProp;
}

void UCPLabCarriedIngredientWidget::UnbindPreviewIngredient()
{
	PreviewIngredient.Reset();
}
