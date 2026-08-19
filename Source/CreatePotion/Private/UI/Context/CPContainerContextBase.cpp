// CPContainerContextBase.h

#include "UI/Context/CPContainerContextBase.h"
#include "Character/CPPlayerController.h"
#include "UI/Widgets/Common/Container/CPItemSlotWidget.h"
#include "Components/CPInventoryComponent.h"

bool UCPContainerContextBase::HandleLeftClickOnly(UCPItemSlotWidget* ClickedSlot)
{
	return false;
}

bool UCPContainerContextBase::HandleLeftDoubleClick(UCPItemSlotWidget* ClickedSlot)
{
	return false;
}

bool UCPContainerContextBase::IsUsingInventoryOnly(ACPPlayerController* PC) const
{
	return PC && PC->CurrentInteractingContainer == nullptr;
}

bool UCPContainerContextBase::IsSlotFromInventory(UCPItemSlotWidget* ClickedSlot, ACPPlayerController* PC) const
{
	return ClickedSlot && ClickedSlot->OwnerContainer
		&& PC && (ClickedSlot->OwnerContainer == PC->CachedInventoryComponent);
}
