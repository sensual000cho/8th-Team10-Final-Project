// CPContainerContextBase.h

#include "UI/Context/CPContainerContextBase.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Character/CPPlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

#include "CreatePotion.h"
#include "UI/Widgets/Common/Container/CPGridSlotWidgetBase.h"
#include "UI/Widgets/Common/Container/CPItemSlotWidget.h"
#include "UI/Widgets/Common/Container/Shop/CPShopPurchasePopupWidget.h"
#include "UI/Widgets/Common/Container/CPItemDetailPopupWidget.h"

#include "Components/CPInventoryComponent.h"
#include "Components/CPDropContainerComponent.h"

#include "Data/CPForageableItemData.h"
#include "Lab/Actor/CPAlchemyProp.h"

#include "Subsystems/Level/DropItemWorldSubsystem.h"

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
        if (!ItemSlot || !ItemSlot->CachedItemData.Instance.SourceItemData)
        {
            return false; // 빈 슬롯이면 false를 리턴하며 아무 것도 하지 않음
        }

        FContainerItem PoppedItem;
        if (ClickedSlot->OwnerContainer->PopItemFromContainer(ClickedSlot->SlotGridIndex, PoppedItem))
        {
            PC->BeginHoldingItem(PoppedItem, ClickedSlot->OwnerContainer, ClickedSlot->SlotGridIndex);
        }
        return true;
    }
    else // 무언가 들고 있었으면
    {
        if (ClickedSlot->OwnerContainer->TargetContext == EUITargetContext::Drop) // Drop 컨테이너일 경우
        {
            // 변신 중에는 드랍을 차단하고 커서에 든 아이템을 유지한다.
            const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PC->GetPawn());
            if (ASC && ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Slime")))) return true;

            FContainerItem& HeldItem = PC->LeftClickPickedContainer->ContainerItems[0];

            if (HeldItem.Instance.IsValid())
            {
                SpawnDroppedProp(HeldItem.Instance.SourceItemData, 1, PC->GetPawn(), HeldItem.Instance.CurrentEffects);
                HeldItem.Stacked -= 1;
            }

            // 버린 이후에 개수가 0이면 
            if (HeldItem.Stacked <= 0)
            {
                PC->ResetHoldingItem();
            }
            else // 0이 아니면
            {
                PC->LeftClickPickedContainer->OnContainerUpdated.Broadcast(); // 남은 수량으로 손 아이콘 갱신
            }

            return true;
        }

        bool bIsPlacedAll = ClickedSlot->OwnerContainer->TryPlaceHoldingItem(PC->LeftClickPickedContainer, ClickedSlot->ClickedGridIndex);
        // 기존 아이템과 스왑 또는 배치하는 함수

        // 다 내려놓았을 땐 PC의 "현재 들고 있는 정보"를 초기화
        if (bIsPlacedAll) 
        {
            PC->ResetHoldingItem();
        }

        return true;
    }
}

bool UCPContainerContextBase::HandleLeftDoubleClick(UCPGridSlotWidgetBase* ClickedSlot)
{
    UE_LOG(LogContainer, Log, TEXT("Left double Click"));
    return true;
}

bool UCPContainerContextBase::HandleCtrlLeftClick(UCPGridSlotWidgetBase* ClickedSlot)
{
    ACPPlayerController* PC = Cast<ACPPlayerController>(ClickedSlot->GetOwningPlayer());
    if (!PC || !PC->CurrentInteractingContainer)
    {
        return false;
    }

    if (!IsSlotFromInventory(ClickedSlot, PC))
    {
        return false;
    }

    UCPItemSlotWidget* ItemSlot = Cast<UCPItemSlotWidget>(ClickedSlot);
    if (!ItemSlot || !ItemSlot->CachedItemData.Instance.SourceItemData)
    {
        return false;
    }

    UCPItemContainerComponent* TargetContainer = PC->CurrentInteractingContainer;

    if (TargetContainer->TargetContext == EUITargetContext::Shop)
    {
        if (UCPShopPurchasePopupWidget* Popup = PC->GetOrCreateShopPopup())
        {
            Popup->OpenForSale(ClickedSlot->OwnerContainer, ClickedSlot->SlotGridIndex, TargetContainer, PC->CachedInventoryComponent);
        }
        return true;
    }

    return ClickedSlot->OwnerContainer->AutoInsertItemToTargetContainer(ClickedSlot->SlotGridIndex, TargetContainer);
}

bool UCPContainerContextBase::HandleShiftLeftClick(UCPGridSlotWidgetBase* ClickedSlot)
{
    ACPPlayerController* PC = Cast<ACPPlayerController>(ClickedSlot->GetOwningPlayer());
    if (!PC || PC->IsHoldingItem())
    {
        return false; // 이미 들고 있으면 나누기 불가
    }

    UCPItemSlotWidget* ItemSlot = Cast<UCPItemSlotWidget>(ClickedSlot);
    if (!ItemSlot || !ItemSlot->CachedItemData.Instance.IsValid() || ItemSlot->CachedItemData.Stacked < 2)
    {
        return false; // 빈 칸이거나 1개짜리는 나누기 기능 제외
    }

    if (UCPShopPurchasePopupWidget* Popup = PC->GetOrCreateShopPopup())
    {
        Popup->OpenForSplit(ClickedSlot->OwnerContainer, ClickedSlot->SlotGridIndex);
    }
    return true;
}

bool UCPContainerContextBase::HandleAltLeftClick(UCPGridSlotWidgetBase* ClickedSlot)
{
    ACPPlayerController* PC = Cast<ACPPlayerController>(ClickedSlot->GetOwningPlayer());
    if (!PC) return false;

    // 수량을 차감하기 전에 변신 상태의 드랍 입력을 차단한다.
    const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PC->GetPawn());
    if (ASC && ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Slime")))) return true;

    if (!IsUsingInventoryOnly(PC))
    {
        return false;
    }

    UCPItemSlotWidget* ItemSlot = Cast<UCPItemSlotWidget>(ClickedSlot);
    if (!ItemSlot || !ItemSlot->CachedItemData.Instance.SourceItemData)
    {
        return false; // 빈 슬롯
    }

    // 삭제되기 전에 미리 값으로 떠둠 (RemoveItemFromContainer가 배열 항목을 지울 수 있으므로)
    UCPForageableItemData* ItemData = ItemSlot->CachedItemData.Instance.SourceItemData;
    TArray<FGameplayTag> EffectTags = ItemSlot->CachedItemData.Instance.CurrentEffects;

    // 스택 전체가 아니라 딱 1개만 차감
    if (!ClickedSlot->OwnerContainer->RemoveItemFromContainer(ClickedSlot->SlotGridIndex, 1))
    {
        return false;
    }

    // TODO[Container] : 일단은 1개만 버리도록
    SpawnDroppedProp(ItemData, 1, PC->GetPawn(), EffectTags);

    return true;
}

bool UCPContainerContextBase::TryEquipItemAt(UCPItemContainerComponent* Container, int32 GridIndex)
{
    return false;
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

void UCPContainerContextBase::SpawnDroppedProp(UCPForageableItemData* ItemData, int32 Count, AActor* NearActor, const TArray<FGameplayTag>& EffectTags) const
{
    if (!ItemData || !NearActor || Count <= 0)
    {
        return;
    }

    // TSoftReference를 실제 메모리에 올리기
    TSubclassOf<ACPThrowablePropBase> PropClass = ItemData->AlchemyPropClass.LoadSynchronous();
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

    // TODO[Container] : 현재는 Count 개수만큼 반복해서 버리도록 설정되어 있고, Count는 1개만 버리도록 하드코딩하고 있음
    // 
    // TODO[Container] : 포션까지 Prop화 되기 위해서는 ACPAlchemyProp 대신 ACPThrowablePropBase를 이용하고
    // InitializeFromItemData의 인자에 EffectTags를 추가로 전달하도록 리팩토링 해야함
    // 
    // Prop에 Count 개수가 반영되면 추후 일괄 버리기 기능으로 Count 개수가 합쳐진 1개의 Prop만 드롭되도록 
    bool bPlayedDropSound = false;
    for (int32 i = 0; i < Count; ++i)
    {
        FVector SpawnLoc = BaseLocation + FVector(FMath::RandRange(-50.f, 50.f), FMath::RandRange(-50.f, 50.f), 10.f);

        if (ACPThrowablePropBase* SpawnedProp = World->SpawnActor<ACPThrowablePropBase>(PropClass, SpawnLoc, FRotator::ZeroRotator))
        {
            SpawnedProp->InitializeFromItemData(ItemData, EffectTags);
            if (!bPlayedDropSound)
            {
                const UCPInventoryComponent* Inventory = NearActor->FindComponentByClass<UCPInventoryComponent>();
                if (Inventory && Inventory->ItemDropSound) UGameplayStatics::PlaySoundAtLocation(NearActor, Inventory->ItemDropSound, SpawnLoc);
                bPlayedDropSound = true;
            }

            // TODO[Container] : 아이템을 버렸을 때 자동 수거 기능을 활성화 할 지 여부 미정
            // if (UDropItemWorldSubsystem* DropSubsystem = World->GetSubsystem<UDropItemWorldSubsystem>())
            // {
            //     DropSubsystem->RegisterDroppedProp(SpawnedProp);
            // }
        }
    }
}

bool UCPContainerContextBase::TrySellItemToShop(UCPItemContainerComponent* PlayerContainer, int32 SlotGridIndex, UCPItemContainerComponent* ShopContainer, UCPInventoryComponent* PlayerInventory) const
{
    if (!PlayerContainer || !ShopContainer || !PlayerInventory)
    {
        return false;
    }

    FContainerItem* Item = PlayerContainer->ContainerItems.FindByPredicate(
        [SlotGridIndex](const FContainerItem& It) { return It.GridIndex == SlotGridIndex; });

    if (!Item || !Item->Instance.SourceItemData)
    {
        return false;
    }

    UCPForageableItemData* ItemData = Item->Instance.SourceItemData;
    int32 SellCount = Item->Stacked;             // 스택 전체를 한 번에 판매
    int32 SellPricePerUnit = ItemData->GetSellPrice();

    // 판매 전에 값 복사 (RemoveItemFromContainer가 배열 항목을 지울 수 있으므로)
    FCPItemInstance InstanceCopy = Item->Instance;

    int32 Leftover = ShopContainer->TryGetItemFromInstance(InstanceCopy, SellCount);
    int32 ActuallySold = SellCount - Leftover;

    if (ActuallySold <= 0)
    {
        return false; // 상점 재고 칸이 가득 차서 하나도 못 넘긴 경우
    }

    PlayerContainer->RemoveItemFromContainer(SlotGridIndex, ActuallySold);
    PlayerInventory->TryGetMoney(SellPricePerUnit * ActuallySold);

    return true;
}

bool UCPContainerContextBase::HandleShowItemDetail(UCPGridSlotWidgetBase* ClickedSlot) const
{
    ACPPlayerController* PC = Cast<ACPPlayerController>(ClickedSlot->GetOwningPlayer());
    if (!PC)
    {
        return false;
    }

    UCPItemSlotWidget* ItemSlot = Cast<UCPItemSlotWidget>(ClickedSlot);
    if (!ItemSlot || !ItemSlot->CachedItemData.Instance.IsValid())
    {
        return false; // 빈 슬롯이면 표시할 게 없음
    }

    if (UCPItemDetailPopupWidget* Popup = PC->GetOrCreateItemDetailPopup())
    {
        Popup->ShowDetail(ItemSlot->CachedItemData.Instance);
        return true;
    }

    return false;
}
