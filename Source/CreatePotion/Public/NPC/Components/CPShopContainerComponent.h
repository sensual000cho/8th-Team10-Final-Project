// CPShopContainerComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/CPItemContainerComponent.h"
#include "CPShopContainerComponent.generated.h"

UCLASS()
class CREATEPOTION_API UCPShopContainerComponent : public UCPItemContainerComponent
{
	GENERATED_BODY()

public:
	UCPShopContainerComponent();
	
protected:
	virtual void BeginPlay() override;
};
