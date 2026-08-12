// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseFixedWidget.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPLabIngredientSelectWidget.generated.h"

class UButton;
class UCPItemContainerComponent;
class UCPContainerMainWidget;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPLabIngredientSelectWidget : public UCPBasePopupWidget
{
	GENERATED_BODY()
	
public:
	void NativeConstruct() override;

protected:
	// 바인딩
	UPROPERTY(meta=(BindWidget))
	UButton* Button_Confirm;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, Category = "UI")
	UCPContainerMainWidget* InventoryWidget;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite, Category = "UI")
	UCPContainerMainWidget* SelectSlotWidget;
	
};
