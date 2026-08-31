// CPItemContainerComponent.h

#include "Components/CPItemContainerComponent.h"
#include "CreatePotion.h"   // 로그용 헤더
#include "Data/CPForageableItemData.h"

UCPItemContainerComponent::UCPItemContainerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPItemContainerComponent::BeginPlay()
{
	Super::BeginPlay();
}

/*
int32 UCPItemContainerComponent::TryGetItem(UCPForageableItemData* InItemData, int32 Count)
{
    if (!InItemData || Count <= 0)
    {
        UE_LOG(LogContainer, Warning, TEXT("TryGetItem 실패: 아이템 데이터가 없거나 수량이 0 이하입니다."));
        return Count;
    }

    // 이미 기존 아이템이 존재하는지 확인 후, 존재한다면 Stack
    for (FContainerItem& Item : ContainerItems)
    {
        if (Item.ItemDataAsset == InItemData && Item.Stacked < MaxStack)
        {
            int32 SpaceLeft = MaxStack - Item.Stacked;  // Stack 가능한 숫자
            int32 AmountToAdd = FMath::Min(Count, SpaceLeft);
            int32 OldStacked = Item.Stacked;

            Item.Stacked += AmountToAdd;
            Count -= AmountToAdd;   // 남은 수량

            UE_LOG(LogContainer, Log, TEXT("아이템 겹치기: [%s (%dx%d)] GridIndex [%d]에 %d개 추가 (%d -> %d) / 현재 [%d / %d]"),
                *InItemData->DisplayName.ToString(),
                InItemData->ContainerSizeX, InItemData->ContainerSizeY,
                Item.GridIndex, AmountToAdd, OldStacked, Item.Stacked, Item.Stacked, MaxStack);

            if (Count <= 0) // 남은 수량이 없으면 = 완벽하게 다 Stack되었으면
            {
                OnContainerUpdated.Broadcast(); // 컨테이너 UI 업데이트
                return 0;
            }
        }
    }

    // 남은 수량이 있다면 새로운 칸에 분할해서 넣기
    while (Count > 0)
    {
        int32 AmountToAdd = FMath::Min(Count, MaxStack);
        int32 FoundIndex = -1;
        bool bHasSpace = false;
        bool bIsRotated = false; // 회전 여부

        // 컨테이너 타입에 따라 빈 공간을 찾는 방식을 다르게 적용
        if (ContainerType == EContainerType::Grid2D)
        {
            FoundIndex = FindGridSpace(InItemData, bIsRotated);

            // -1이 아니면 자리를 찾은 것
            if (FoundIndex != -1)
            {
                bHasSpace = true;
            }
        }
        else if (ContainerType == EContainerType::Slot1D)
        {
            bHasSpace = FindSlotSpace(FoundIndex);
        }

        // 공간이 충분하면 아이템을 컨테이너에 넣기
        if (bHasSpace)
        {
            FContainerItem NewItem;
            NewItem.Instance.SourceItemData = InItemData;
            NewItem.Stacked = AmountToAdd;
            NewItem.GridIndex = FoundIndex;
            NewItem.bIsRotated = bIsRotated;

            ContainerItems.Add(NewItem);
            Count -= AmountToAdd;

            UE_LOG(LogContainer, Log, TEXT("새 아이템 획득 성공! [%s (%dx%d)] GridIndex [%d]에 %d개 위치함 (회전: %d) 현재 총 %d개"),
                *InItemData->DisplayName.ToString(),
                InItemData->ContainerSizeX, InItemData->ContainerSizeY,
                FoundIndex, AmountToAdd, bIsRotated, ContainerItems.Num());
        }
        else
        {
            UE_LOG(LogContainer, Warning, TEXT("컨테이너 공간 부족으로 인해 [%s] 남은 %d개를 반환"),
                *InItemData->DisplayName.ToString(), Count);
            break;
        }
    }

    // UI 업데이트 Broadcast
    OnContainerUpdated.Broadcast();
    return Count;
}
*/

int32 UCPItemContainerComponent::TryGetItem(UCPForageableItemData* InItemData, int32 Count)
{
    FCPItemInstance Instance;
    Instance.SourceItemData = InItemData;
    return TryGetItem(Instance, Count);
}

int32 UCPItemContainerComponent::TryGetItem(const FCPItemInstance& InInstance, int32 Count)
{
    return int32();
}

bool UCPItemContainerComponent::IsGridSpaceEnough(int32 TargetIndex, int32 ItemWidth, int32 ItemHeight) const
{
    if (TargetIndex < 0)
    {
        return false;
    }
    // 넣고자 하는 목표 위치의 2D 좌표 (X, Y)
    int32 TargetCol = TargetIndex % Columns;
    int32 TargetRow = TargetIndex / Columns;

    // 컨테이너 경계선 검사
    if (TargetCol + ItemWidth > Columns || TargetRow + ItemHeight > Rows)
    {
        return false; // 컨테이너 밖으로 아이템이 넘치면 불가능하다고 알리기
    }

    // 컨테이너 내 기존 아이템과 충돌 검사
    for (const FContainerItem& ExistingItem : ContainerItems)
    {
        if (!ExistingItem.Instance.SourceItemData || ExistingItem.GridIndex < 0) continue;

        // 기존 아이템의 위치 (X, Y)
        int32 ExCol = ExistingItem.GridIndex % Columns;
        int32 ExRow = ExistingItem.GridIndex / Columns;

        // 기존 아이템의 가로/세로 (회전되어 있다면 Width와 Height를 바꿔서 계산)
        int32 ExW = ExistingItem.bIsRotated ? ExistingItem.Instance.SourceItemData->ContainerSizeY : ExistingItem.Instance.SourceItemData->ContainerSizeX;
        int32 ExH = ExistingItem.bIsRotated ? ExistingItem.Instance.SourceItemData->ContainerSizeX : ExistingItem.Instance.SourceItemData->ContainerSizeY;

        // Overlap 판별
        bool bOverlapX = (TargetCol < ExCol + ExW) && (TargetCol + ItemWidth > ExCol);
        bool bOverlapY = (TargetRow < ExRow + ExH) && (TargetRow + ItemHeight > ExRow);

        // X축도 겹치고 Y축 둘 다 Overlap될 때
        if (bOverlapX && bOverlapY)
        {
            return false;
        }
    }

    // 경계선도 안 넘고, 겹치는 아이템도 없다면 true를 반환하여 빈 공간임을 알림
    return true;
}

int32 UCPItemContainerComponent::FindItemArrayIndexCoveringGridIndex(int32 QueryIndex) const
{
    if (QueryIndex < 0 || ContainerType != EContainerType::Grid2D)
    {
        // Slot1D는 애초에 칸 하나=아이템 하나라 기존 방식(정확히 일치)이면 충분
        return ContainerItems.IndexOfByPredicate(
            [QueryIndex](const FContainerItem& It) { return It.GridIndex == QueryIndex; });
    }

    int32 QueryCol = QueryIndex % Columns;
    int32 QueryRow = QueryIndex / Columns;

    for (int32 i = 0; i < ContainerItems.Num(); ++i)
    {
        const FContainerItem& Item = ContainerItems[i];
        if (!Item.Instance.SourceItemData || Item.GridIndex < 0) continue;

        int32 ItemCol = Item.GridIndex % Columns;
        int32 ItemRow = Item.GridIndex / Columns;
        int32 ItemW = Item.bIsRotated ? Item.Instance.SourceItemData->ContainerSizeY : Item.Instance.SourceItemData->ContainerSizeX;
        int32 ItemH = Item.bIsRotated ? Item.Instance.SourceItemData->ContainerSizeX : Item.Instance.SourceItemData->ContainerSizeY;

        // 클릭한 칸이 이 아이템의 바운딩 박스 안에 들어오는지
        if (QueryCol >= ItemCol && QueryCol < ItemCol + ItemW &&
            QueryRow >= ItemRow && QueryRow < ItemRow + ItemH)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

int32 UCPItemContainerComponent::FindGridSpace(UCPForageableItemData* ItemData, bool& bOutIsRotated)
{
    if (!ItemData)
    {
        return -1;
    }

    int32 ItemW = ItemData->ContainerSizeX;
    int32 ItemH = ItemData->ContainerSizeY;
    int32 TotalSlots = Columns * Rows;

    // 정방향으로 가방 맨 앞부터 끝까지 빈 곳이 있나 찾기
    for (int32 i = 0; i < TotalSlots; ++i)
    {
        bool bIsGridEnough = IsGridSpaceEnough(i, ItemW, ItemH);
        if (bIsGridEnough)
        {
            bOutIsRotated = false;
            return i; // Grid Index를 반환하여 해당 Index에 아이템을 배치하라고 Return
        }
    }

    // 만약 정방향 자리가 없고 가로/세로 길이가 다른 아이템이라면
    if (ItemW != ItemH)
    {
        for (int32 i = 0; i < TotalSlots; ++i)
        {
            // ItemW와 ItemH의 자리를 바꿔서(회전해서) 검사
            if (IsGridSpaceEnough(i, ItemH, ItemW))
            {
                bOutIsRotated = true; // 회전해서 넣어야 한다고 회전값을 true로 변경
                return i; // Grid Index를 반환하여 해당 Index에 아이템을 배치하라고 Return
            }
        }
    }

    // 모든 공간을 찾았는데도 자리가 없으면 실패
    bOutIsRotated = false;
    return -1; // Grid Index가 -1이면 자리가 없는 경우 = 임시 인벤토리로
}

bool UCPItemContainerComponent::FindSlotSpace(int32& OutGridIndex)
{
    // 0번 슬롯부터 MaxSlots-1 번 슬롯까지 차례로 확인
    for (int32 SlotIndex = 0; SlotIndex < MaxSlots; ++SlotIndex)
    {
        bool bIsOccupied = false;

        // 현재 인벤토리 아이템 중에 이 번호를 차지하고 있는 게 있는지 검사
        for (const FContainerItem& Item : ContainerItems)
        {
            if (Item.GridIndex == SlotIndex)
            {
                bIsOccupied = true;
                break;
            }
        }

        // 차지한 아이템이 없다면
        if (!bIsOccupied)
        {
            OutGridIndex = SlotIndex;
            return true;
        }
    }

    return false; // 모든 슬롯이 꽉 찼을 경우
}

bool UCPItemContainerComponent::RemoveItemFromContainer(int32 TargetGridIndex, int32 AmountToRemove) 
{
    for (int32 i = 0; i < ContainerItems.Num(); ++i)
    {
        if (ContainerItems[i].GridIndex == TargetGridIndex)
        {
            // 가진 개수가 뺄 개수보다 많거나 같을 때만 허용
            if (ContainerItems[i].Stacked >= AmountToRemove)
            {
                ContainerItems[i].Stacked -= AmountToRemove;

                // 만약 다 빼서 0개가 되었다면 배열에서 삭제
                if (ContainerItems[i].Stacked <= 0)
                {
                    ContainerItems.RemoveAt(i);
                }

                // UI 갱신 Broadcast
                OnContainerUpdated.Broadcast();
                return true;
            }
            break;
        }
    }
    return false;
}

bool UCPItemContainerComponent::PopItemFromContainer(int32 TargetGridIndex, FContainerItem& OutPoppedItem)
{
    int32 ArrayIdx = ContainerItems.IndexOfByPredicate(
        [TargetGridIndex](const FContainerItem& It) { return It.GridIndex == TargetGridIndex; });

    if (ArrayIdx == INDEX_NONE)
    {
        return false;
    }

    OutPoppedItem = ContainerItems[ArrayIdx];
    ContainerItems.RemoveAt(ArrayIdx);
    OnContainerUpdated.Broadcast(); // 집는 즉시 화면에서 빈 칸으로 보이도록 UI 업데이트를 위한 Broadcast
    return true;
}

bool UCPItemContainerComponent::AutoInsertItemToTargetContainer(int32 SourceGridIndex, UCPItemContainerComponent* TargetContainer)
{
    // 옮기려는 컨테이너가 유효하지 않거나 자기 자신에게 옮기는 시도라면 무시
    if (!TargetContainer || TargetContainer == this)
    {
        UE_LOG(LogContainer, Log, TEXT("대상 컨테이너가 유효하지 않습니다."))
        return false;
    }

    // 옮길 아이템 찾기
    FContainerItem* ItemToMove = nullptr;
    for (FContainerItem& Item : ContainerItems)
    {
        if (Item.GridIndex == SourceGridIndex)
        {
            ItemToMove = &Item;
            break;
        }
    }

    // 아이템이 없거나 데이터가 비어있으면 실패
    if (!ItemToMove || !ItemToMove->Instance.SourceItemData)
    {
        return false;
    }

    // 안전하게 데이터 복사해두기 (배열이 수정될 수 있으므로)
    UCPForageableItemData* DataAsset = ItemToMove->Instance.SourceItemData;
    int32 OriginalCount = ItemToMove->Stacked;

    // 대상(Target) 컨테이너에 아이템 밀어 넣기
    int32 LeftoverCount = TargetContainer->TryGetItem(DataAsset, OriginalCount);
    int32 TransferredCount = OriginalCount - LeftoverCount;

    // 1개 이상이 성공적으로 넘어갔다면 갯수 차감
    if (TransferredCount > 0)
    {
        RemoveItemFromContainer(SourceGridIndex, TransferredCount);
        UE_LOG(LogContainer, Log, TEXT("[%s] 로 아이템 이동 성공 (총 %d개 중 %d개 이동)"), 
            *DataAsset->DisplayName.ToString(), OriginalCount, TransferredCount);
        return true;
    }

    UE_LOG(LogContainer, Warning, TEXT("[%s] 대상 컨테이너 공간 부족"), 
        *DataAsset->DisplayName.ToString());
    return false;
}

bool UCPItemContainerComponent::TryPlaceHoldingItem(UCPItemContainerComponent* HandContainer, int32 TargetIndex)
{
    UE_LOG(LogContainer, Warning, TEXT("[Place] 시도: HandContainer 아이템 수=%d, TargetIndex=%d"),
        HandContainer->ContainerItems.Num(), TargetIndex);
    if (!HandContainer || HandContainer->ContainerItems.Num() == 0)
    {
        return false;
    }

    FContainerItem HeldItem = HandContainer->ContainerItems[0];
    int32 ItemW = HeldItem.bIsRotated ? HeldItem.Instance.SourceItemData->ContainerSizeY : HeldItem.Instance.SourceItemData->ContainerSizeX;
    int32 ItemH = HeldItem.bIsRotated ? HeldItem.Instance.SourceItemData->ContainerSizeX : HeldItem.Instance.SourceItemData->ContainerSizeY;

    // 바운딩 박스 기준으로 "이 칸을 덮고 있는 아이템"을 찾음
    int32 TargetArrayIdx = FindItemArrayIndexCoveringGridIndex(TargetIndex);
    bool bTargetEmpty = (TargetArrayIdx == INDEX_NONE);

    // Case A : 빈 칸
    if (bTargetEmpty)
    {
        bool bCanPlace = (ContainerType == EContainerType::Grid2D)
            ? IsGridSpaceEnough(TargetIndex, ItemW, ItemH)
            : (TargetIndex < MaxSlots);

        if (!bCanPlace)
        {
            return false;
        }

        FContainerItem NewItem = HeldItem;
        NewItem.GridIndex = TargetIndex;
        ContainerItems.Add(NewItem);
        OnContainerUpdated.Broadcast();

        HandContainer->ContainerItems.Empty();
        HandContainer->OnContainerUpdated.Broadcast();
        return true;
    }

    FContainerItem ExistingItem = ContainerItems[TargetArrayIdx];

    // Case B : 같은 타입 -> 스택 합치기 (기존 로직 그대로, 위치 무관)
    if (UCPItemContainerComponent::AreInstancesStackable(ExistingItem.Instance, HeldItem.Instance))
    {
        int32 SpaceLeft = MaxStack - ExistingItem.Stacked;
        int32 AmountToMove = FMath::Min(HeldItem.Stacked, SpaceLeft);
        if (AmountToMove <= 0) return false;

        ContainerItems[TargetArrayIdx].Stacked += AmountToMove;
        int32 Remaining = HeldItem.Stacked - AmountToMove;
        OnContainerUpdated.Broadcast();

        if (Remaining <= 0)
        {
            HandContainer->ContainerItems.Empty();
            HandContainer->OnContainerUpdated.Broadcast();
            return true;
        }
        else
        {
            HandContainer->ContainerItems[0].Stacked = Remaining;
            HandContainer->OnContainerUpdated.Broadcast();
            return false;
        }
    }
    // Case C : Swap
    else
    {
        // 기존 아이템을 먼저 배열에서 완전히 제거
        ContainerItems.RemoveAt(TargetArrayIdx);

        // 기존 아이템이 사라진 상태에서 클릭한 위치(TargetIndex)에 Holding 아이템이 들어갈 수 있는지 검사
        bool bCanPlace = (ContainerType == EContainerType::Grid2D)
            ? IsGridSpaceEnough(TargetIndex, ItemW, ItemH)
            : (TargetIndex < MaxSlots);

        if (!bCanPlace)
        {
            // 자리가 안 나오면 방금 지운 기존 아이템을 그대로 복구하고 실패 처리
            ContainerItems.Add(ExistingItem);
            return false;
        }

        // (bCanPlace == true)배치가 가능하다면 Holding 아이템을 클릭한 자리에 배치
        FContainerItem NewItem = HeldItem;
        NewItem.GridIndex = TargetIndex;
        ContainerItems.Add(NewItem);
        OnContainerUpdated.Broadcast();

        // 이제 Hand로 기존 아이템을 Swap
        ExistingItem.GridIndex = 0;
        HandContainer->ContainerItems[0] = ExistingItem;
        HandContainer->OnContainerUpdated.Broadcast();
        return false; // 손은 안 비었음 - 대신 기존 아이템을 들게 됨
    }
}

bool UCPItemContainerComponent::AreInstancesStackable(const FCPItemInstance& A, const FCPItemInstance& B)
{
    if (A.SourceItemData != B.SourceItemData)
    {
        return false;
    }

    if (A.CurrentEffects.Num() != B.CurrentEffects.Num())
    {
        return false;
    }

    for (const FGameplayTag& Tag : A.CurrentEffects)
    {
        if (!B.CurrentEffects.Contains(Tag))
        {
            return false;
        }
    }

    // 모든 비교 후 동일하다면 Stack 가능하다고 true 리턴
    return true;
}
