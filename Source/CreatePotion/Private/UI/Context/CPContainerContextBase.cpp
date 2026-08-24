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

        FContainerItem PoppedItem;
        if (ClickedSlot->OwnerContainer->PopItemFromContainer(ClickedSlot->SlotGridIndex, PoppedItem))
        {
            UE_LOG(LogContainer, Warning, TEXT("[Pick] 집기 성공: %s, 손 컨테이너 개수: %d"),
                *PoppedItem.ItemDataAsset->DisplayName.ToString(), PC->LeftClickPickedContainer->ContainerItems.Num());

            PoppedItem.GridIndex = 0; // 손-컨테이너는 칸이 1개뿐
            PC->LeftClickPickedContainer->ContainerItems.Add(PoppedItem);
            PC->LeftClickPickedContainer->OnContainerUpdated.Broadcast();

            // 취소 시 되돌아갈 기존 Container의 종류와 index 정보 저장
            PC->LeftClickPickedItemOriginContainer = ClickedSlot->OwnerContainer;
            PC->LeftClickPickedOriginSlotIndex = ClickedSlot->SlotGridIndex;
        }

        // TODO[Container] : 커서에 아이콘을 붙이는 드래그 비주얼 구현
        return true;
    }
    else // 무언가 들고 있었으면
    {
        bool bIsPlacedAll = ClickedSlot->OwnerContainer->TryPlaceHoldingItem(
            PC->LeftClickPickedContainer, ClickedSlot->ClickedGridIndex);
        // 기존 아이템과 스왑 또는 배치하는 함수

        // 다 내려놓았을 땐 PC의 "현재 들고 있는 정보"를 초기화
        if (bIsPlacedAll) 
        {
            PC->ResetHoldingItem();
        }

        // TODO: 마우스 커서를 원래대로 복구하는 로직 위치 (하드웨어 커서 복구 또는 커서 위젯 숨기기)
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
