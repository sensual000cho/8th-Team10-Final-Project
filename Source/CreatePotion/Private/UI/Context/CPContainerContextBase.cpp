// CPContainerContextBase.h

#include "UI/Context/CPContainerContextBase.h"
#include "Character/CPPlayerController.h"

#include "CreatePotion.h"
#include "UI/Widgets/Common/Container/CPGridSlotWidgetBase.h"
#include "UI/Widgets/Common/Container/CPItemSlotWidget.h"
#include "Components/CPInventoryComponent.h"
#include "Components/CPDropContainerComponent.h"

#include "Data/CPForageableItemData.h"
#include "Lab/Actor/CPAlchemyProp.h"

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
            UE_LOG(LogContainer, Warning, TEXT("[Pick] 집기 성공: %s, Hand 컨테이너 개수: %d"),
                *PoppedItem.ItemDataAsset->DisplayName.ToString(), PC->LeftClickPickedContainer->ContainerItems.Num());

            PoppedItem.GridIndex = 0; // Hand 컨테이너는 1칸만 존재
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

bool UCPContainerContextBase::HandleAltLeftClick(UCPGridSlotWidgetBase* ClickedSlot)
{
    ACPPlayerController* PC = Cast<ACPPlayerController>(ClickedSlot->GetOwningPlayer());
    if (!PC) return false;

    if (!IsUsingInventoryOnly(PC))
    {
        return false;
    }

    UCPItemSlotWidget* ItemSlot = Cast<UCPItemSlotWidget>(ClickedSlot);
    if (!ItemSlot || !ItemSlot->CachedItemData.ItemDataAsset)
    {
        return false; // 빈 슬롯
    }

    // 삭제되기 전에 미리 값으로 떠둠 (RemoveItemFromContainer가 배열 항목을 지울 수 있으므로)
    UCPForageableItemData* ItemData = ItemSlot->CachedItemData.ItemDataAsset;

    // 스택 전체가 아니라 딱 1개만 차감
    if (!ClickedSlot->OwnerContainer->RemoveItemFromContainer(ClickedSlot->SlotGridIndex, 1))
    {
        return false;
    }

    // TODO[Container] : 일단은 1개만 버리도록
    SpawnDroppedProp(ItemData, 1, PC->GetPawn());

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

void UCPContainerContextBase::SpawnDroppedProp(UCPForageableItemData* ItemData, int32 Count, AActor* NearActor) const
{
    if (!ItemData || !NearActor || Count <= 0)
    {
        return;
    }

    // TSoftReference를 실제 메모리에 올리기
    TSubclassOf<ACPAlchemyProp> PropClass = ItemData->AlchemyPropClass.LoadSynchronous();
    if (!PropClass)
    {
        UE_LOG(LogContainer, Warning, TEXT("[Drop] [%s]의 AlchemyPropClass가 설정되지 않음"),
            *ItemData->DisplayName.ToString());
        return;
    }

    UWorld* World = NearActor->GetWorld();
    if (!World)
    {
        return;
    }

    FVector BaseLocation = NearActor->GetActorLocation() + NearActor->GetActorForwardVector() * 100.f;

    // TODO[Container] : 현재는 Count 개수만큼 반복해서 버리도록 설정되어 있음
    // Prop에 Count 개수가 반영되면 추후 일괄 버리기 기능으로 Count 개수가 합쳐진 1개의 Prop만 드롭되도록 
    for (int32 i = 0; i < Count; ++i)
    {
        FVector SpawnLoc = BaseLocation + FVector(FMath::RandRange(-50.f, 50.f), FMath::RandRange(-50.f, 50.f), 10.f);

        if (ACPAlchemyProp* SpawnedProp = World->SpawnActor<ACPAlchemyProp>(PropClass, SpawnLoc, FRotator::ZeroRotator))
        {
            SpawnedProp->InitializeFromItemData(ItemData);
        }
    }
}
