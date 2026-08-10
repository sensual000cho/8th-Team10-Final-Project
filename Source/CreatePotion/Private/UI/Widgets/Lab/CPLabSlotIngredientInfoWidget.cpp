#include "UI/Widgets/Lab/CPLabSlotIngredientInfoWidget.h"

#include "GameState/CPLabGameState.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

#define LOCTEXT_NAMESPACE "CPLabSlotIngredientInfoWidget"

void UCPLabSlotIngredientInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 공용 부모는 Visibility를 결정하지 않는다. 슬롯 카드는 실제 호버가 시작될 때만 보인다.
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCPLabSlotIngredientInfoWidget::BeginSlotHover(int32 InSlotIndex)
{
	if (InSlotIndex == INDEX_NONE)
	{
		HoveredSlotIndex = INDEX_NONE;
		HideSlotInfo();
		return;
	}

	HoveredSlotIndex = InSlotIndex;
	RefreshHoveredSlot();
}

void UCPLabSlotIngredientInfoWidget::EndSlotHover(int32 InSlotIndex)
{
	// 슬롯 A 종료보다 슬롯 B 시작이 먼저 처리될 수 있으므로, 현재 슬롯과 다른 종료 요청은 무시한다.
	if (HoveredSlotIndex != InSlotIndex)
	{
		return;
	}

	HoveredSlotIndex = INDEX_NONE;
	HideSlotInfo();
}

void UCPLabSlotIngredientInfoWidget::BindEvents()
{
	Super::BindEvents();

	if (UWorld* World = GetWorld())
	{
		if (ACPLabGameState* LabGameState = World->GetGameState<ACPLabGameState>())
		{
			BoundPotionSession = LabGameState->GetPotionSession();
			if (BoundPotionSession)
			{
				BoundPotionSession->OnSessionChanged.AddUniqueDynamic(
					this,
					&UCPLabSlotIngredientInfoWidget::HandleSessionChanged);
			}
		}
	}
}

void UCPLabSlotIngredientInfoWidget::UnbindEvents()
{
	if (BoundPotionSession)
	{
		BoundPotionSession->OnSessionChanged.RemoveDynamic(
			this,
			&UCPLabSlotIngredientInfoWidget::HandleSessionChanged);
		BoundPotionSession = nullptr;
	}

	HoveredSlotIndex = INDEX_NONE;
	Super::UnbindEvents();
}

void UCPLabSlotIngredientInfoWidget::HandleSessionChanged()
{
	// 호버 중에 슬롯의 재료가 교체돼도 카드를 닫지 않고 같은 슬롯의 최신 재료로 갱신한다.
	if (HoveredSlotIndex != INDEX_NONE)
	{
		RefreshHoveredSlot();
	}
}

void UCPLabSlotIngredientInfoWidget::RefreshHoveredSlot()
{
	if (HoveredSlotIndex == INDEX_NONE || !IsValid(BoundPotionSession))
	{
		HideSlotInfo();
		return;
	}

	ACPAlchemyProp* IngredientProp = nullptr;
	if (!BoundPotionSession->GetIngredientPropFromSlot(HoveredSlotIndex, IngredientProp) ||
		!IsValid(IngredientProp))
	{
		HideSlotInfo();
		return;
	}

	SetHeaderText(FText::Format(
		LOCTEXT("SlotHeader", "슬롯 {0}"),
		FText::AsNumber(HoveredSlotIndex + 1)));
	SetObservedIngredient(IngredientProp);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCPLabSlotIngredientInfoWidget::HideSlotInfo()
{
	// 먼저 Prop 델리게이트를 해제한 뒤 숨겨야 보이지 않는 카드가 이전 재료를 계속 관찰하지 않는다.
	ClearObservedIngredient();
	SetVisibility(ESlateVisibility::Collapsed);
}

#undef LOCTEXT_NAMESPACE
