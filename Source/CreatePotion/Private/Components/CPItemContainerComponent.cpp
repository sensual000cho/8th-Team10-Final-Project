// CPItemContainerComponent.h

#include "Components/CPItemContainerComponent.h"
#include "CreatePotion.h"                               // 로그용 헤더
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
        return Count; // int32 반환형에 맞게 반환
    }

    // 이미 기존 아이템이 존재하는지 확인 후 Stack
    for (FContainerItem& Item : ContainerItems)
    {
        // 같은 아이템이면서, MaxStack개가 안 된 슬롯을 찾음
        if (Item.ItemDataAsset == InItemData && Item.Stacked < MaxStack)
        {
            int32 SpaceLeft = MaxStack - Item.Stacked;
            int32 AmountToAdd = FMath::Min(Count, SpaceLeft);
            int32 OldStacked = Item.Stacked;

            Item.Stacked += AmountToAdd;
            Count -= AmountToAdd; // 넣은 만큼 남은 수량 차감

            // [아이템 이름] / (n -> m) / 현재 [m/10]
            UE_LOG(LogContainer, Log, TEXT("아이템 겹치기: [%s] GridIndex [%d]에 %d개 추가 (%d -> %d) / 현재 [%d / %d]"),
                *InItemData->DisplayName.ToString(), Item.GridIndex, AmountToAdd, OldStacked, Item.Stacked, Item.Stacked, MaxStack);

            if (Count <= 0)
            {
                // TODO: UI 업데이트 델리게이트 호출
                return 0; // 성공적으로 다 넣었으므로 남은 수량 0 반환
            }
        }
    }

    // 남은 수량이 있다면 새로운 칸에 분할해서 넣기
    int32 ItemWidth = 1;  // InItemData->SizeX; 
    int32 ItemHeight = 1; // InItemData->SizeY;

    // 남은 개수가 0이 될 때까지 계속 빈 공간을 찾음
    while (Count > 0)
    {
        int32 AmountToAdd = FMath::Min(Count, MaxStack);
        int32 FoundIndex = -1;
        bool bHasSpace = false;

        // 컨테이너 타입에 따라 빈 공간을 찾는 방식을 다르게 적용!
        if (ContainerType == EContainerType::Grid2D)
        {
            bHasSpace = FindGridSpace(ItemWidth, ItemHeight, FoundIndex);
        }
        else if (ContainerType == EContainerType::Slot1D)
        {
            bHasSpace = FindSlotSpace(FoundIndex);
        }

        if (bHasSpace)
        {
            FContainerItem NewItem;
            NewItem.ItemDataAsset = InItemData;
            NewItem.Stacked = AmountToAdd;
            NewItem.GridIndex = FoundIndex;

            ContainerItems.Add(NewItem);
            Count -= AmountToAdd;

            UE_LOG(LogContainer, Log, TEXT("새 아이템 획득 성공! GridIndex [%d]에 %d개 위치함. 현재 컨테이너 아이템 총 %d개"),
                FoundIndex, AmountToAdd, ContainerItems.Num());
        }
        else
        {
            // 공간이 꽉 차서 더 이상 넣을 수 없으면 루프 탈출
            UE_LOG(LogContainer, Warning, TEXT("컨테이너 공간 부족! 다 넣지 못한 남은 %d개를 반환합니다."), Count);
            break;
        }
    }

    // TODO: UI 업데이트 델리게이트 호출

    // 전부 다 들어갔다면 0, 공간이 부족해서 남았다면 남은 Count 반환
    return Count;
}

bool UCPItemContainerComponent::FindGridSpace(int32 ItemWidth, int32 ItemHeight, int32& OutGridIndex)
{
    int32 TotalSlots = Columns * Rows;
    TArray<bool> OccupiedGrid;
    OccupiedGrid.Init(false, TotalSlots);

    // 현재 인벤토리에 있는 아이템들을 바탕으로 '사용 중'인 칸을 모두 true로 칠함
    for (const FContainerItem& Item : ContainerItems)
    {
        if (!Item.ItemDataAsset || Item.GridIndex < 0) continue;

        // 아이템의 가로 세로 크기
        // 회전 여부에 따라 Width와 Height를 바꿔줌, false일 때 원래대로
        // int32 W = Item.bIsRotated ? ItemHeight : ItemWidth;
        // int32 H = Item.bIsRotated ? ItemWidth : ItemHeight;
        int32 W = Item.bIsRotated ? 1 : 1;
        int32 H = Item.bIsRotated ? 1 : 1;

        int32 StartX = Item.GridIndex % Columns;
        int32 StartY = Item.GridIndex / Columns;

        for (int32 y = 0; y < H; ++y)
        {
            for (int32 x = 0; x < W; ++x)
            {
                int32 MarkIndex = (StartY + y) * Columns + (StartX + x);
                if (OccupiedGrid.IsValidIndex(MarkIndex))
                {
                    OccupiedGrid[MarkIndex] = true;
                }
            }
        }
    }

    // 왼쪽 위부터 빈 공간을 찾기 시작
    for (int32 y = 0; y <= Rows - ItemHeight; ++y)
    {
        for (int32 x = 0; x <= Columns - ItemWidth; ++x)
        {
            bool bCanFit = true;

            // (x, y) 위치에 아이템(Width x Height)을 놓을 수 있는지 검사
            for (int32 checkY = 0; checkY < ItemHeight; ++checkY)
            {
                for (int32 checkX = 0; checkX < ItemWidth; ++checkX)
                {
                    int32 CheckIndex = (y + checkY) * Columns + (x + checkX);
                    if (OccupiedGrid[CheckIndex] == true)
                    {
                        bCanFit = false;
                        break; // 겹치는 칸 발견 시 즉시 중단
                    }
                }
                if (!bCanFit) break;
            }

            // 공간을 찾았다면 해당 위치의 인덱스를 반환하고 종료
            if (bCanFit)
            {
                OutGridIndex = y * Columns + x;
                return true;
            }
        }
    }

    // 모든 칸을 확인했지만 공간이 없을 때 = 임시 인벤토리 생성
    return false;
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
