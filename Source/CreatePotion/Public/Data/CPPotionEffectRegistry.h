#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Items/Potion/Effect/CPPotionEffectHandler.h"
#include "CPPotionEffectRegistry.generated.h"

// 대상별 효과 태그와 Handler Class 연결을 보관하는 Registry DataAsset
UCLASS(BlueprintType)
class CREATEPOTION_API UCPPotionEffectRegistry : public UDataAsset
{
	GENERATED_BODY()

public:
	// Context의 효과 태그를 순서대로 조회해 하나 이상 적용되면 true 반환
	UFUNCTION(BlueprintCallable, Category = "Potion|Effect")
	bool TryApplyPotionEffects(const FCPPotionImpactContext& Context) const;

	// 값이 None이면 해당 대상에 명시적으로 적용하지 않음
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Effect")
	TMap<FGameplayTag, TSubclassOf<UCPPotionEffectHandler>> EffectHandlers;
};
