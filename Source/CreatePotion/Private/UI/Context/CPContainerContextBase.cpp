// CPContainerContextBase.h

#include "UI/Context/CPContainerContextBase.h"
#include "Character/CPPlayerController.h"

#include "CreatePotion.h"
#include "UI/Widgets/Common/Container/CPItemSlotWidget.h"
#include "Components/CPInventoryComponent.h"

bool UCPContainerContextBase::HandleLeftClickOnly(UCPItemSlotWidget* ClickedSlot)
{
    ACPPlayerController* PC = Cast<ACPPlayerController>(ClickedSlot->GetOwningPlayer());
    if (!PC)
    {
        return false;
    }

    // 무언가 들고 있는게 없으면
    if (!PC->IsHoldingItem())
    {
        if (!ClickedSlot->CachedItemData.ItemDataAsset)
        {
            return false; // 빈 슬롯이면 false를 리턴하며 아무 것도 하지 않음
        }

        // 아이템 잡기
        PC->LeftClickPickedContainer = ClickedSlot->OwnerContainer;
        PC->LeftClickPickedSlotIndex = ClickedSlot->CachedItemData.GridIndex;
        // TODO[Container] : 커서에 아이콘을 붙이는 드래그 비주얼 구현
        return true;
    }
    else // 무언가 들고 있었으면
    {
        // TODO: PickedContainer에서 ClickedSlot->OwnerContainer로 아이템 이동/스왑 로직 호출
        UE_LOG(LogContainer, Log, TEXT("아이템 내려놓기/스왑 시도, 출발지 Index: %d -> 목적지 Index: %d"), 
            PC->LeftClickPickedSlotIndex, ClickedSlot->CachedItemData.GridIndex);

        // TODO[Container] : 기존 아이템과 스왑 또는 배치하는 기능 구현
        // PC->PickedContainer->MoveOrSwapItem(PC->PickedSlotIndex, ClickedSlot->OwnerContainer, ClickedSlot->CachedItemData.GridIndex);

        // 다 내려놓았으니 PC의 상태를 초기화
        PC->ResetHoldingItem();

        // TODO: 마우스 커서를 원래대로 복구하는 로직 (하드웨어 커서 복구 또는 커서 위젯 숨기기)
        return true;
    }
}

bool UCPContainerContextBase::HandleLeftDoubleClick(UCPItemSlotWidget* ClickedSlot)
{
	UE_LOG(LogContainer, Log, TEXT("Left double Click"));
	return true;
}

bool UCPContainerContextBase::IsUsingInventoryOnly(ACPPlayerController* PC) const
{
	return PC && PC->CurrentInteractingContainer == nullptr;
}

bool UCPContainerContextBase::IsSlotFromInventory(UCPItemSlotWidget* ClickedSlot, ACPPlayerController* PC) const
{
	return ClickedSlot && ClickedSlot->OwnerContainer
		&& PC && (ClickedSlot->OwnerContainer == PC->CachedInventoryComponent);
}
