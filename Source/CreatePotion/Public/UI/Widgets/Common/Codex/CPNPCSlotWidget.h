// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPNPCSlotWidget.generated.h"

class UTextBlock;
class UButton;
class UCPNPCDataAsset;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPNPCSlotWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void BindEvents() override;
	virtual void UnbindEvents() override;
	
	void SetNPCData(UCPNPCDataAsset* InNPCData);
	
private:
	UFUNCTION()
	void HandleSlotClicked();
	
	void RefreshSlot();
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_NPC;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_NPCName;
	
	UPROPERTY()
	TObjectPtr<UCPNPCDataAsset> NPCData;
};
