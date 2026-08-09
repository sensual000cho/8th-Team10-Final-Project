// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	ECPInputMode InputMode = ECPInputMode::GameAndUI;
	
	ECPInputMode GetInputMode() const { return InputMode; }
	//ECPInputMode SetInputMode() { return ECPInputMode::GameAndUI; }
	
	UFUNCTION(BlueprintCallable, Category = "Popup")
	void RequestClose();
	
	// 타이머 설정
	UFUNCTION(BlueprintCallable, Category = "Popup")
	void SetAutoClose(float Duration);
	
protected:
	FTimerHandle AutoCloseTimerHandle;
};


