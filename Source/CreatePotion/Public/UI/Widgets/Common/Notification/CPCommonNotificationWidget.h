// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPCommonNotificationWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPCommonNotificationWidget : public UCPBasePopupWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Notification")
	void SetNotifiactionText(const FText& InText);
	
	// 자동 소멸 시간 ( 0 = 자동 소멸 안 함 )
	UFUNCTION(BlueprintCallable, Category = "Notification")
	void ShowWithAutoClose(const FText& InText, float Duration = 2.0f);
	
protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextBlock_Notification;
	
	FTimerHandle AutoCloseTimerHandle;
};
