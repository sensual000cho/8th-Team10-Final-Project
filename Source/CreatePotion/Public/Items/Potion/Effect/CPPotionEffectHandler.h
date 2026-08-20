#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Items/Potion/CPPotionImpactContext.h"
#include "CPPotionEffectHandler.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class CREATEPOTION_API UCPPotionEffectHandler : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Potion|Effect")
	bool TryApplyPotionEffect(FGameplayTag EffectTag, const FCPPotionImpactContext& Context);

	virtual bool TryApplyPotionEffect_Implementation(FGameplayTag EffectTag, const FCPPotionImpactContext& Context);
};
