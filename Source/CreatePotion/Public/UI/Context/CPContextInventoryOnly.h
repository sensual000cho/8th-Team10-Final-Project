// CPContextInventoryOnly.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Context/CPContainerContextBase.h"
#include "CPContextInventoryOnly.generated.h"

UCLASS()
class CREATEPOTION_API UCPContextInventoryOnly : public UCPContainerContextBase
{
	GENERATED_BODY()
	
public:
	virtual bool HandleLeftClickOnly(UCPItemSlotWidget* ClickedSlot) override;
};
