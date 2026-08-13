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
	
	// 위젯을 여는 함수
	UFUNCTION(BlueprintCallable, Category = "UI|Debug")
	UUserWidget* PushWidget(TSubclassOf<UUserWidget> WidgetClass);
	
	
	// 특정 위젯을 지정해서 닫기 (순서무관)
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseWidget(UUserWidget* Widget);
	
	// 가장 위에 열려있는 위젯을 닫기
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseTopWidget();
	
	// 위젯을 가장 위로 가져오기
	UFUNCTION(BlueprintCallable, Category = "UI")
	void BringWidgetToFront(UUserWidget* Widget);
	
	// 위젯 토글 함수
	UFUNCTION(BlueprintCallable, Category = "UI")
	UUserWidget* ToggleWidget(TSubclassOf<UUserWidget> WidgetClass);
	
	// 해당 위젯이 열려있는지 확인 후 해당 위젯 인스턴스 반환
	UFUNCTION(BlueprintCallable, Category = "UI")
	UUserWidget* FindOpenWidget(TSubclassOf<UUserWidget> WidgetClass);
private:
	struct FPopupEntry
	{
		UUserWidget* Widget = nullptr;
		ECPInputMode InputMode = ECPInputMode::GameAndUI;
	};
	
	void UpdateInputMode(); 
	
	TArray<FPopupEntry> OpenWidgets;
	
public:
	void RegisterPushedWidget(UUserWidget* Widget);
};


