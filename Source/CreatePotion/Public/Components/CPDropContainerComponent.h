// CPDropContainerComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/CPItemContainerComponent.h"
#include "CPDropContainerComponent.generated.h"

UCLASS(ClassGroup = (CPContainer), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPDropContainerComponent : public UCPItemContainerComponent
{
	GENERATED_BODY()

public:
	UCPDropContainerComponent();

};
