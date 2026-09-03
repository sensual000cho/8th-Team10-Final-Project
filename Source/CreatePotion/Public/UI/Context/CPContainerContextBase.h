// CPContainerContextBase.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CPContainerContextBase.generated.h"

class UCPGridSlotWidgetBase;
class ACPPlayerController;
class UCPForageableItemData;

UCLASS()
class CREATEPOTION_API UCPContainerContextBase : public UObject
{
	GENERATED_BODY()
	
public:
	// TODO : 마우스 오버레이 시 기능( 아이템 설명 UI 출력 등 )

	virtual bool HandleLeftClickOnly(UCPGridSlotWidgetBase* ClickedSlot);
	virtual bool HandleLeftDoubleClick(UCPGridSlotWidgetBase* ClickedSlot);
	virtual bool HandleCtrlLeftClick(UCPGridSlotWidgetBase* ClickedSlot) { return false; }
	virtual bool HandleShiftLeftClick(UCPGridSlotWidgetBase* ClickedSlot) { return false; }
	virtual bool HandleAltLeftClick(UCPGridSlotWidgetBase* ClickedSlot);

	virtual bool HandleRightClickOnly(UCPGridSlotWidgetBase* ClickedSlot) { return false; }
	virtual bool HandleCtrlRightClick(UCPGridSlotWidgetBase* ClickedSlot) { return false; }
	virtual bool HandleShiftRightClick(UCPGridSlotWidgetBase* ClickedSlot) { return false; }
	virtual bool HandleAltRightClick(UCPGridSlotWidgetBase* ClickedSlot) { return false; }

	UFUNCTION(BlueprintCallable, Category = "Container")
	virtual bool TryEquipItemAt(UCPItemContainerComponent* Container, int32 GridIndex);

protected:
	bool IsUsingInventoryOnly(ACPPlayerController* PC) const;
	bool IsSlotFromInventory(UCPGridSlotWidgetBase* ClickedSlot, ACPPlayerController* PC) const;

	virtual void SpawnDroppedProp(UCPForageableItemData* ItemData, int32 Count, AActor* NearActor) const;
};
