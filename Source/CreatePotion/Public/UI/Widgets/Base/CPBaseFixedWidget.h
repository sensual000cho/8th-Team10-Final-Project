// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPBaseFixedWidget.generated.h"

/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPBaseFixedWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()
	
protected:
	UFUNCTION(BlueprintCallable, Category = "Fixed")
	void ShowWidget();
	
	UFUNCTION(BlueprintCallable, Category = "Fixed")
	void HideWidget();
	
	UFUNCTION(BlueprintPure, Category = "Fixed")
	bool IsWidgetShown() const;
};
