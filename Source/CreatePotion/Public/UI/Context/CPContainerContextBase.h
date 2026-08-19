// CPContainerContextBase.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CPContainerContextBase.generated.h"

class UCPItemSlotWidget;
class ACPPlayerController;

UCLASS()
class CREATEPOTION_API UCPContainerContextBase : public UObject
{
	GENERATED_BODY()
	
public:
	// TODO : 마우스 오버레이 시 기능( 아이템 설명 UI 출력 등 )

	virtual bool HandleLeftClickOnly(UCPItemSlotWidget* ClickedSlot) { return false; }
	virtual bool HandleLeftDoubleClick(UCPItemSlotWidget* ClickedSlot) { return false; }
	virtual bool HandleCtrlLeftClick(UCPItemSlotWidget* ClickedSlot) { return false; }
	virtual bool HandleShiftLeftClick(UCPItemSlotWidget* ClickedSlot) { return false; }
	virtual bool HandleAltLeftClick(UCPItemSlotWidget* ClickedSlot) { return false; }

	virtual bool HandleRightClickOnly(UCPItemSlotWidget* ClickedSlot) { return false; }
	virtual bool HandleCtrlRightClick(UCPItemSlotWidget* ClickedSlot) { return false; }
	virtual bool HandleShiftRightClick(UCPItemSlotWidget* ClickedSlot) { return false; }
	virtual bool HandleAltRightClick(UCPItemSlotWidget* ClickedSlot) { return false; }

protected:
	bool IsUsingInventoryOnly(ACPPlayerController* PC) const;
	bool IsSlotFromInventory(UCPItemSlotWidget* ClickedSlot, ACPPlayerController* PC) const;
};
