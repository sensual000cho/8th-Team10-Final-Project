// CPHandItemContainerComponent.cpp

#pragma once

#include "CoreMinimal.h"
#include "Components/CPItemContainerComponent.h"
#include "CPHandItemContainerComponent.generated.h"

UCLASS(ClassGroup = (CPContainer), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPHandItemContainerComponent : public UCPItemContainerComponent
{
	GENERATED_BODY()

public:
	UCPHandItemContainerComponent();
	
};
