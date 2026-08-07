// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPBasePopupWidget.generated.h"

/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPBasePopupWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool RequiresUIFocus() const {return true;}
	
	UFUNCTION(BlueprintCallable, Category = "Popup")
	void RequestClose();
};
