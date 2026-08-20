// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Items/Potion/CPPotionImpactContext.h"
#include "CPPotionEffectReceiver.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UCPPotionEffectReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CREATEPOTION_API ICPPotionEffectReceiver
{
	GENERATED_BODY()

public:
	// 실제로 효과를 받아들였을 때만 true를 반환한다.
	UFUNCTION(BlueprintNativeEvent)
	bool ReceivePotionImpact(const FCPPotionImpactContext& Context);
};
