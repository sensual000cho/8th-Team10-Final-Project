// CPDropContainerComponent.cpp

#include "Components/CPDropContainerComponent.h"

UCPDropContainerComponent::UCPDropContainerComponent()
{
	ContainerType = EContainerType::Slot1D;
	MaxSlots = 9999;
	ContainerUIClass = nullptr; // UI를 그릴 필요가 없음
}
