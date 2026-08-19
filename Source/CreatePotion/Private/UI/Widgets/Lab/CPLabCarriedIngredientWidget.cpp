#include "UI/Widgets/Lab/CPLabCarriedIngredientWidget.h"

#include "Character/CPInteractionComponent.h"
#include "GameFramework/Pawn.h"
#include "GameState/CPLabGameState.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

void UCPLabCarriedIngredientWidget::NativeConstruct()
{
	// Super 호출 중 UCPBaseUserWidget이 가상 BindEvents를 호출하므로 세션 바인딩은 이미 끝난 상태다.
	Super::NativeConstruct();

	SetHeaderText(FText::FromString(TEXT("현재 운반중인 재료")));
	SetVisibility(ESlateVisibility::HitTestInvisible);
	HandlePropChanged();
}

void UCPLabCarriedIngredientWidget::BindEvents()
{
	Super::BindEvents();

	// 상호작용 주체를 직접 찾지 않고 GameState가 소유한 세션을 운반 상태의 단일 출처로 사용한다.
	if (UWorld* World = GetWorld())
	{
		if (ACPLabGameState* LabGameState = World->GetGameState<ACPLabGameState>())
		{
			BoundPotionSession = LabGameState->GetPotionSession();
			if (BoundPotionSession)
			{
				BoundPotionSession->OnHeldAlchemyPropChanged.AddUniqueDynamic(
					this,
					&UCPLabCarriedIngredientWidget::HandlePropChanged);
			}
		}
	}

	if (APawn* OwningPawn = GetOwningPlayerPawn())
	{
		BoundInteractionComponent = OwningPawn->FindComponentByClass<UCPInteractionComponent>();
		if (BoundInteractionComponent.IsValid())
		{
			BoundInteractionComponent->OnPromptChanged.AddUniqueDynamic(
				this,
				&UCPLabCarriedIngredientWidget::HandleInteractionFocusChanged);

			if (AActor* CurrentTarget = BoundInteractionComponent->GetCurrentTarget())
			{
				(void)CurrentTarget;
				ClearPreviewEffects();
			}
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

	if (BoundPotionSession)
	{
		BoundPotionSession->OnHeldAlchemyPropChanged.RemoveDynamic(
			this,
			&UCPLabCarriedIngredientWidget::HandlePropChanged);
		BoundPotionSession = nullptr;
	}

	Super::UnbindEvents();
}

void UCPLabCarriedIngredientWidget::HandlePropChanged()
{
	// 슬롯 호버 카드와 달리 운반 카드는 빈손이어도 "비어 있음" 상태로 계속 표시한다.
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (!BoundPotionSession)
	{
		UnbindPreviewIngredient();
		ClearObservedIngredient();
		ClearPreviewEffects();
		return;
	}

	ACPAlchemyProp* HeldIngredientProp = BoundPotionSession->GetHeldAlchemyProp();
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
