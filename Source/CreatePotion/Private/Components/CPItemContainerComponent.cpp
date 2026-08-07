// CPItemContainerComponent.h

#include "Components/CPItemContainerComponent.h"
#include "CreatePotion.h"   // 로그용 헤더
#include "Data/CPForageableItemData.h"

UCPItemContainerComponent::UCPItemContainerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCPItemContainerComponent::BeginPlay()
{
	Super::BeginPlay();
}

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
            NewItem.ItemDataAsset = InItemData;
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
        if (!ExistingItem.ItemDataAsset || ExistingItem.GridIndex < 0) continue;

        // 기존 아이템의 위치 (X, Y)
        int32 ExCol = ExistingItem.GridIndex % Columns;
        int32 ExRow = ExistingItem.GridIndex / Columns;

        // 기존 아이템의 가로/세로 (회전되어 있다면 Width와 Height를 바꿔서 계산)
        int32 ExW = ExistingItem.bIsRotated ? ExistingItem.ItemDataAsset->ContainerSizeY : ExistingItem.ItemDataAsset->ContainerSizeX;
        int32 ExH = ExistingItem.bIsRotated ? ExistingItem.ItemDataAsset->ContainerSizeX : ExistingItem.ItemDataAsset->ContainerSizeY;

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
