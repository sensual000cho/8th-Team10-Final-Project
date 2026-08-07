// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CPUIManagerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	
	// 위젯 띄우는 템플릿
	template<typename T>
	T* PushWidget(TSubclassOf<T> WidgetClass, bool bRequiresUIFocus = true)
	{
		T* Widget = CreateWidget<T>(GetWorld(), WidgetClass);
		Widget->AddToViewport(OpenWidgets.Num());
		OpenWidgets.Add({Widget, bRequiresUIFocus});
		return Widget;
	}
	
	// 특정 위젯을 지정해서 닫기 (순서무관)
	UFUNCTION(BlueprintCallable, Category = "UI|Debug")
	void CloseWidget(UUserWidget* Widget);
	
	void CloseTopWidget();
private:
	struct FPopupEntry
	{
		UUserWidget* Widget = nullptr;
		bool bRequiresUIFocus = true;  
	};
	TArray<FPopupEntry> OpenWidgets;
	void UpdateInputMode(); 
	
public:
	// 디버그용 BP함수
	UFUNCTION(BlueprintCallable, Category = "UI|Debug")
	UUserWidget* PushWidgetBP(TSubclassOf<UUserWidget> WidgetClass);
	
};
