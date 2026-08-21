// CPContainerContextBase.h

#include "UI/Context/CPContainerContextBase.h"
#include "Character/CPPlayerController.h"

#include "CreatePotion.h"
#include "UI/Widgets/Common/Container/CPGridSlotWidgetBase.h"
#include "UI/Widgets/Common/Container/CPItemSlotWidget.h"
#include "Components/CPInventoryComponent.h"

bool UCPContainerContextBase::HandleLeftClickOnly(UCPGridSlotWidgetBase* ClickedSlot)
{
    ACPPlayerController* PC = Cast<ACPPlayerController>(ClickedSlot->GetOwningPlayer());
    if (!PC)
    {
        return false;
    }


    if (!PC->IsHoldingItem()) // 무언가 들고 있는게 없으면
    {
        // UCPGridSlotWidgetBase를 UCPItemSlotWidget로 다운캐스팅 시도
        UCPItemSlotWidget* ItemSlot = Cast<UCPItemSlotWidget>(ClickedSlot);
        
        // (다운캐스팅 실패 or 다운캐스팅 이후에 ItemDA가 없으면) = 빈 슬롯이면
        if (!ItemSlot || !ItemSlot->CachedItemData.ItemDataAsset)
        {
            return false; // 빈 슬롯이면 false를 리턴하며 아무 것도 하지 않음
        }

        // 아이템 집기 처리
        PC->LeftClickPickedContainer = ClickedSlot->OwnerContainer;
        PC->LeftClickPickedSlotIndex = ClickedSlot->GridIndex;
        // TODO[Container] : 커서에 아이콘을 붙이는 드래그 비주얼 구현
        return true;
    }
    else // 무언가 들고 있었으면
    {
        // 기존 아이템과 스왑 또는 배치하는 함수
        PC->LeftClickPickedContainer->MoveOrSwapItem(PC->LeftClickPickedSlotIndex, ClickedSlot->OwnerContainer, ClickedSlot->GridIndex);

        // 다 내려놓았으니 PC의 상태를 초기화
        PC->ResetHoldingItem();

        // TODO: 마우스 커서를 원래대로 복구하는 로직 (하드웨어 커서 복구 또는 커서 위젯 숨기기)
        return true;
    }
}

bool UCPContainerContextBase::HandleLeftDoubleClick(UCPGridSlotWidgetBase* ClickedSlot)
{
	UE_LOG(LogContainer, Log, TEXT("Left double Click"));
	return true;
}

bool UCPContainerContextBase::IsUsingInventoryOnly(ACPPlayerController* PC) const
{
	return PC && PC->CurrentInteractingContainer == nullptr;
}

bool UCPContainerContextBase::IsSlotFromInventory(UCPGridSlotWidgetBase* ClickedSlot, ACPPlayerController* PC) const
{
	return ClickedSlot && ClickedSlot->OwnerContainer
		&& PC && (ClickedSlot->OwnerContainer == PC->CachedInventoryComponent);
}
