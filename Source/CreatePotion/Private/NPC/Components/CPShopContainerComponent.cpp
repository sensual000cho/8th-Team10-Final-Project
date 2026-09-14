// CPShopContainerComponent.cpp

#include "NPC/Components/CPShopContainerComponent.h"

UCPShopContainerComponent::UCPShopContainerComponent()
{
	ContainerType = EContainerType::Slot1D;
	MaxSlots = 20;
	TargetContext = EUITargetContext::Shop;
}

void UCPShopContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	// GridIndex를 배열 순서 그대로 자동 배정
	for (int32 i = 0; i < ContainerItems.Num(); ++i)
	{
		ContainerItems[i].GridIndex = i;
	}
}
