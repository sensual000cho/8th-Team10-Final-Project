// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPCodexMainWidget.generated.h"

class UCPForageableItemData;
class UCPForageableCodexWidget;
class UCPCodexSubsystem;
class UWidgetSwitcher;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPCodexMainWidget : public UCPBasePopupWidget
{
	GENERATED_BODY()
	
protected:
	void BindEvents() override;
	void UnbindEvents() override;

private:
	UFUNCTION()
	void OpenItemDetail(UCPForageableItemData* SelectedItemData);

public:
	// 아이템 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Codex")
	TSubclassOf<UUserWidget> ForageableCodexWidgetClass;

protected:
	// 도감 서브시스템
	UPROPERTY(BlueprintReadOnly, Category = "Codex")
	TObjectPtr<UCPCodexSubsystem> CodexSubsystem;
	
private:
	// --- 위젯 바인딩 ---
	UPROPERTY(meta=(BindWidget, AllowPrivateAccess = "true"), BlueprintReadWrite, Category = "Codex")
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCPForageableCodexWidget> WBP_CPForageableCodex;
};
