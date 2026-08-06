// CPInventoryComponent.cpp

#include "Components/CPInventoryComponent.h"
#include "CreatePotion.h"	// 로그용

int32 UCPInventoryComponent::TryGetItem(UCPForageableItemData* InItemData, int32 Count)
{
	int32 LeftoverCount = Super::TryGetItem(InItemData, Count);

	// 가방이 꽉 차서 남은 아이템이 반환되었을 때 임시 인벤토리에 저장
	if (LeftoverCount > 0)
	{
		FContainerItem TempItem;
		TempItem.ItemDataAsset = InItemData;
		TempItem.Stacked = LeftoverCount;
		TempItem.GridIndex = -1; // 임시 인벤토리임을 나타내는 인덱스

		TempInventoryItems.Add(TempItem);

		UE_LOG(LogContainer, Warning, TEXT("남은 %d개가 임시 인벤토리로 이동되었습니다."), LeftoverCount);

		// UI 쪽에 임시 인벤토리 팝업창을 띄우라고 Broadcast
		OnTempInventoryOpened.Broadcast();

		// 임시 인벤토리가 남은 수량을 모두 흡수했으므로, 
		// "더 이상 필드에 떨어뜨릴 아이템은 없다"는 의미로 0을 반환
		return 0;
	}

	// 남은 게 없었다면 그대로 0 반환
	return 0;
}
