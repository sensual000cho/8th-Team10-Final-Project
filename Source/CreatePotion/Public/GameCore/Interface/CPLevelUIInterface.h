// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "CPLevelUIInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UCPLevelUIInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CREATEPOTION_API ICPLevelUIInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "UI")
	void TogglePopup(FGameplayTag PopupTag);
};
