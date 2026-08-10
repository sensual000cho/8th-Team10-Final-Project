// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CPUIManagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class ECPInputMode : uint8
{
	GameOnly	UMETA(DisplayName = "Game Only"),
	UIOnly		UMETA(DisplayName = "UI Only"),
	GameAndUI	UMETA(DisplayName = "Game and UI"),
};

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
	T* PushWidget(TSubclassOf<T> WidgetClass);
	
	// 특정 위젯을 지정해서 닫기 (순서무관)
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseWidget(UUserWidget* Widget);
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseTopWidget();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void BringWidgetToFront(UUserWidget* Widget);
	
	
	
private:
	struct FPopupEntry
	{
		UUserWidget* Widget = nullptr;
		ECPInputMode InputMode = ECPInputMode::GameAndUI;
	};
	
	void UpdateInputMode(); 
	
	TArray<FPopupEntry> OpenWidgets;
	
public:
	// 디버그용 BP함수
	UFUNCTION(BlueprintCallable, Category = "UI|Debug")
	UUserWidget* PushWidgetBP(TSubclassOf<UUserWidget> WidgetClass);
	
};


