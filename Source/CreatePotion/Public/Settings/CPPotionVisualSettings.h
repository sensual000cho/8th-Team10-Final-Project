// CPPotionVisualSettings.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "CPPotionVisualSettings.generated.h"

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Potion Visual Settings"))
class CREATEPOTION_API UCPPotionVisualSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	// 효과 태그 하나당 대응되는 액체 색을 저장하는 Map
	UPROPERTY(config, EditAnywhere, Category = "Potion")
	TMap<FGameplayTag, FLinearColor> EffectColors;

	// 효과가 없거나 매핑에 없는 태그일 때 사용할 기본 색
	UPROPERTY(config, EditAnywhere, Category = "Potion")
	FLinearColor DefaultLiquidColor = FLinearColor::White;
};
