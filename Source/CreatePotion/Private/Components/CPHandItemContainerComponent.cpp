// CPHandItemContainerComponent.cpp

#include "Components/CPHandItemContainerComponent.h"

UCPHandItemContainerComponent::UCPHandItemContainerComponent()
{
	ContainerType = EContainerType::Slot1D;
	MaxSlots = 1;
	TargetContext = EUITargetContext::Hand;
}
