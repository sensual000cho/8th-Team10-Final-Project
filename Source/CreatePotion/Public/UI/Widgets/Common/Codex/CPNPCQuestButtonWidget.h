// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPNPCQuestButtonWidget.generated.h"

class UTextBlock;
class UButton;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNPCQuestButtonClicked, FName, QuestID);

UCLASS()
class CREATEPOTION_API UCPNPCQuestButtonWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void BindEvents() override;
	virtual void UnbindEvents() override;
	
	void SetQuest(FName InQuestID, const FText& InQuestTitle);
	
private:
	UFUNCTION()
	void HandleButtonClicked();
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Codex|NPC")
	FOnNPCQuestButtonClicked OnNPCQuestButtonClicked;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Quest;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_QuestTitle;
	
	UPROPERTY()
	FName QuestID = NAME_None;
};
