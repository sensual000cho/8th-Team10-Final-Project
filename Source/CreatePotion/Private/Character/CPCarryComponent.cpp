// CPCarryComponent.cpp

#include "Character/CPCarryComponent.h"
#include "Lab/Actor/CPThrowablePropBase.h"

UCPCarryComponent::UCPCarryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UCPCarryComponent::AttachProp(ACPThrowablePropBase* Prop)
{
	if (!IsValid(Prop))
	{
		return false;
	}

	return Prop->AttachAsHeld(this);
}

bool UCPCarryComponent::DetachProp(ACPThrowablePropBase* Prop, const FVector& DropLocation)
{
	if (!IsValid(Prop))
	{
		return false;
	}

	Prop->DetachAsHeld(DropLocation);
	return true;
}

