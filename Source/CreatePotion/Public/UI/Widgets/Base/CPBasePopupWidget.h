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
	
	UFUNCTION(BlueprintCallable, Category = "Popup")
	virtual void RequestClose();
	
	// 타이머 설정
	UFUNCTION(BlueprintCallable, Category = "Popup")
	virtual void SetAutoClose(float Duration);
	
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
protected:
	FTimerHandle AutoCloseTimerHandle;
	
	// 음수면 비활성
	float AutoCloseRemaining = -1.f; 
};


