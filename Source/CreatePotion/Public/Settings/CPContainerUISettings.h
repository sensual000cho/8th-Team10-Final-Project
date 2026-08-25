// CPContainerUISettings.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CPContainerUISettings.generated.h"

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "CreatePotion Container UI Settings"))
class CREATEPOTION_API UCPContainerUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category = "Container|UI")
	float SlotSize = 80.0f;

	UPROPERTY(config, EditAnywhere, Category = "Container|UI")
	float SlotPadding = 5.0f;
	
};
