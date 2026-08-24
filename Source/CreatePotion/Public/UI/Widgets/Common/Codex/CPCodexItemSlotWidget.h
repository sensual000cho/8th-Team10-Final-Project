// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "CPCodexItemSlotWidget.generated.h"

class UCPForageableItemData;
class UButton;
class UImage;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPCodexItemSlotWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()
	
public:
	void BindEvents() override;
	void UnbindEvents() override;

	// --- Set 함수 ---
	void SetItemData(UCPForageableItemData* NewItemData);
	
protected:
	UFUNCTION(BlueprintCallable, Category = "UPCodexItemSlotWidget")
	void HandleSlotClicked();
	
	UFUNCTION(BlueprintCallable, Category = "UPCodexItemSlotWidget")
	void RefreshImage();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UCPForageableItemData> ItemData;

private:
	// --- 위젯 바인딩 ---
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Item;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Item;
	
};

