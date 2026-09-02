// CPInventoryMainWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Common/Container/CPContainerMainWidget.h"
#include "CPInventoryMainWidget.generated.h"

UCLASS()
class CREATEPOTION_API UCPInventoryMainWidget : public UCPContainerMainWidget
{
	GENERATED_BODY()

protected:
	virtual void HandleCloseRequested() override;

};
