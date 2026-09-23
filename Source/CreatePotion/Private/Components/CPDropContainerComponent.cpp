// CPDropContainerComponent.cpp

#include "Components/CPDropContainerComponent.h"

UCPDropContainerComponent::UCPDropContainerComponent()
{
	ContainerType = EContainerType::Slot1D;
	MaxSlots = 1;
	ContainerUIClass = nullptr; // UI를 그릴 필요가 없음
	TargetContext = EUITargetContext::Drop;
}
