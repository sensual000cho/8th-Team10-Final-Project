// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CPHighlightable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UCPHighlightable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CREATEPOTION_API ICPHighlightable
{
	GENERATED_BODY()
	
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 외곽선 하이라이트 설정
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void SetHighLight(bool bHighLighted);
};


