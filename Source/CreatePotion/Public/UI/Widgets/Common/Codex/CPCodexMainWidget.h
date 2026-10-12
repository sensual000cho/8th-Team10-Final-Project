// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPCodexMainWidget.generated.h"

class UCPNPCCodexWidget;
class UCPNPCDataAsset;
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
	
	UFUNCTION()
	void OpenNPCDetail(UCPNPCDataAsset* SelectedNPCData);

public:
	// 아이템 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Codex")
	TSubclassOf<UUserWidget> ForageableCodexWidgetClass;
	
	

protected:
	// 도감 내부 상세 화면 이동음. 위젯 전체 열기/닫기 음원과 별도로 지정한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> NavigationSound;

	// 도감 서브시스템
	UPROPERTY(BlueprintReadOnly, Category = "Codex")
	TObjectPtr<UCPCodexSubsystem> CodexSubsystem;
	
private:
	// --- 위젯 바인딩 ---
	UPROPERTY(meta=(BindWidget, AllowPrivateAccess = "true"), BlueprintReadWrite, Category = "Codex")
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCPForageableCodexWidget> WBP_CPForageableCodex;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCPNPCCodexWidget> WBP_CPNPCCodex;
};
