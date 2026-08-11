// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseFixedWidget.h"
#include "CPLabIngredientSelectWidget.generated.h"

/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPLabIngredientSelectWidget : public UCPBaseFixedWidget
{
	GENERATED_BODY()
	
private:
	// 바인딩
	UPROPERTY(meta=(BindWidget))
	UButton* Button_Confirm;
	
	
};
