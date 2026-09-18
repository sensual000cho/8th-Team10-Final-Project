// CPContainerSettings.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CPContainerSettings.generated.h"

class USoundBase;

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "CreatePotion Container Settings"))
class CREATEPOTION_API UCPContainerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category = "Container|UI")
	float SlotSize = 80.0f;

	UPROPERTY(config, EditAnywhere, Category = "Container|UI")
	float SlotPadding = 5.0f;
};
