// CPInventoryComponent.cpp

#include "Components/CPInventoryComponent.h"
#include "CreatePotion.h"                               // 로그용 헤더
#include "Data/CPForageableItemData.h"

UCPInventoryComponent::UCPInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCPInventoryComponent::TryGetItem(UCPForageableItemData* InItemData, int32 Count)
{
	if (!InItemData || Count <= 0)
	{
        UE_LOG(LogInventory, Warning, TEXT("TryGetItem 실패: 아이템 데이터가 없거나 수량이 0 이하입니다."));
		return;
	}

    // 이미 기존 아이템이 존재하는지 확인 후 겹치는 로직, 참조를 통해 원본 데이터 수정
    for (FInventoryItem& Item : InventoryItems)
    {
        // 같은 아이템이면서, 50개가 안 된 슬롯을 찾음
        if (Item.ItemDataAsset == InItemData && Item.Stacked < MaxStack)
        {
            // 이 칸에 몇 개를 더 넣을 수 있는지 계산
            int32 SpaceLeft = MaxStack - Item.Stacked;
            // 남은 Count와 SpaceLeft 중 더 작은 값만큼만 넣음
            int32 AmountToAdd = FMath::Min(Count, SpaceLeft);

            int32 OldStacked = Item.Stacked;

            Item.Stacked += AmountToAdd;
            Count -= AmountToAdd; // 넣은 만큼 남은 수량 차감

            // [아이템 이름] / (n -> m) / 현재 [m/10]
            UE_LOG(LogInventory, Log, TEXT("아이템 겹치기: [%s] GridIndex [%d]에 %d개 추가 (%d -> %d) / 현재 [%d / %d]"),
                *InItemData->DisplayName.ToString(), Item.GridIndex, AmountToAdd, OldStacked, Item.Stacked, Item.Stacked, MaxStack);

            if (Count <= 0)
            {
                // TODO: UI 업데이트 델리게이트 호출
                return; // 성공적으로 겹쳤으므로 새 공간을 찾을 필요 없이 함수 즉시 종료
            }
        }
    }

	int32 FoundIndex = -1;
                          // TODO : 아이템 크기 조절
	int32 ItemWidth = 1;  // InItemData->SizeX; 
	int32 ItemHeight = 1; // InItemData->SizeY;

	bool bHasSpace = FindAvailableSpace(ItemWidth, ItemHeight, FoundIndex);
	
	if (bHasSpace)
	{
		FInventoryItem NewItem;
		NewItem.ItemDataAsset = InItemData;
		NewItem.Stacked = Count;
		NewItem.GridIndex = FoundIndex;

		InventoryItems.Add(NewItem);
        UE_LOG(LogInventory, Log, TEXT
        ("아이템 획득 성공! GridIndex [%d]에 위치함. 현재 가방 아이템 총 %d개")
            , FoundIndex, InventoryItems.Num());

		// TODO: UI 업데이트 델리게이트 호출
	}
	else
	{
		// 공간이 없다면 '임시 인벤토리'로 이동
		FInventoryItem TempItem;
		TempItem.ItemDataAsset = InItemData;
		TempItem.Stacked = Count;
		TempItem.GridIndex = -1;

		TempInventoryItems.Add(TempItem);
        UE_LOG(LogInventory, Warning, TEXT
        ("인벤토리 공간 부족! 아이템이 임시 인벤토리 대기열로 이동되었습니다. 임시 보관 아이템 수: %d"), 
            TempInventoryItems.Num());

		OnTempInventoryOpened.Broadcast();
	}
}

bool UCPInventoryComponent::FindAvailableSpace(int32 ItemWidth, int32 ItemHeight, int32& OutGridIndex)
{
    int32 TotalSlots = Columns * Rows;
    TArray<bool> OccupiedGrid;
    OccupiedGrid.Init(false, TotalSlots);

    // 현재 인벤토리에 있는 아이템들을 바탕으로 '사용 중'인 칸을 모두 true로 칠함
    for (const FInventoryItem& Item : InventoryItems)
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
