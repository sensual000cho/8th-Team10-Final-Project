// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPNPCCodexWidget.generated.h"

class UCPNPCQuestButtonWidget;
class UImage;
class UScrollBox;
class UTextBlock;
class UCPNPCDataAsset;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPNPCCodexWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Codex|NPC")
	void SetNPCData(UCPNPCDataAsset* InNPCData);
	
private:
	void RefreshCodex();
	void RefreshCompletedQuestButtons();
	void SetQuestDsecription(FName QuestID);	
	
	UFUNCTION()
	void HandleQuestButtonClicked(FName QuestID);
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_NPC;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Name;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_QuestTitle;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Description;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> SB_Quest;
	
	UPROPERTY(EditDefaultsOnly, Category = "Codex|NPC")
	TSubclassOf<UCPNPCQuestButtonWidget> QuestButtonWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UCPNPCDataAsset> CurrentNPCData;
	
	UPROPERTY()
	FName FocusQuestID = NAME_None;
	
	UPROPERTY()
	TArray<TObjectPtr<UCPNPCQuestButtonWidget>> QuestButtons;
};
