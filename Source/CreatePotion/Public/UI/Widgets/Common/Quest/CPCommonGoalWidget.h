// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseFixedWidget.h"
#include "Quest/QuestTypes.h"
#include "CPCommonGoalWidget.generated.h"

class UTextBlock;
class UQuestManager;

UCLASS()
class CREATEPOTION_API UCPCommonGoalWidget : public UCPBaseFixedWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleQuestUpdated(FName QuestID, EQuestState NewState);

	UFUNCTION()
	void HandleInventoryUpdated();

	void UpdateGoalDisplay();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_GoalTitle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_GoalText;
};