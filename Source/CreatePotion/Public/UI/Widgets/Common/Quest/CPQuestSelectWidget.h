// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPQuestSelectWidget.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPQuestSelectWidget : public UCPBasePopupWidget
{
	GENERATED_BODY()
	
protected:
	void BindEvents() override;
	void UnbindEvents() override;
	
	// --- 버튼 콜백 함수 ---
	UFUNCTION()
	void HandleCallButtonClicked();
	
	UFUNCTION()
	void HandleCancelButtonClicked();
	
	// 임시 QuestID 탐색 함수
	bool FirstAcceptedQuestId();
	
protected:
	// --- 버튼 바인딩 ---
	UPROPERTY(meta = (BindWidget))
	UButton* Button_CallCustomer;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Cancel;
	
private:
	// NPC 호출 시 사용할 QuestId;
	FName SelectedQuestId;
};
