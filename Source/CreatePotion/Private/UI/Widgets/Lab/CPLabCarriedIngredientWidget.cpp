#include "UI/Widgets/Lab/CPLabCarriedIngredientWidget.h"

#include "GameState/CPLabGameState.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

#define LOCTEXT_NAMESPACE "CPLabCarriedIngredientWidget"

void UCPLabCarriedIngredientWidget::NativeConstruct()
{
	// Super 호출 중 UCPBaseUserWidget이 가상 BindEvents를 호출하므로 세션 바인딩은 이미 끝난 상태다.
	Super::NativeConstruct();

	SetHeaderText(LOCTEXT("CarryHeader", "현재 운반중인 재료"));
	SetVisibility(ESlateVisibility::HitTestInvisible);
	RefreshHeldIngredient();
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
				BoundPotionSession->OnSessionChanged.AddUniqueDynamic(
					this,
					&UCPLabCarriedIngredientWidget::HandleSessionChanged);
			}
		}
	}
}

void UCPLabCarriedIngredientWidget::UnbindEvents()
{
	if (BoundPotionSession)
	{
		BoundPotionSession->OnSessionChanged.RemoveDynamic(
			this,
			&UCPLabCarriedIngredientWidget::HandleSessionChanged);
		BoundPotionSession = nullptr;
	}

	Super::UnbindEvents();
}

void UCPLabCarriedIngredientWidget::HandleSessionChanged()
{
	RefreshHeldIngredient();
}

void UCPLabCarriedIngredientWidget::RefreshHeldIngredient()
{
	// 슬롯 호버 카드와 달리 운반 카드는 빈손이어도 "비어 있음" 상태로 계속 표시한다.
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (!BoundPotionSession)
	{
		ClearObservedIngredient();
		return;
	}

	ACPAlchemyProp* HeldIngredientProp = BoundPotionSession->GetHeldIngredientProp();
	if (IsValid(HeldIngredientProp))
	{
		SetObservedIngredient(HeldIngredientProp);
		return;
	}

	ClearObservedIngredient();
}

#undef LOCTEXT_NAMESPACE
