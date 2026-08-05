// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseHUDWidget.h"
#include "CPInteractionPromptWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPInteractionPromptWidget : public UCPBaseHUDWidget
{
	GENERATED_BODY()
	
protected:
	virtual void BindEvents() override;
	virtual void UnbindEvents() override;
	
public:
	// 상호작용 대상 탐색 컴포넌트가 호출할 함수
	// bHasTarget: 지금 조준 중인 대상이 있는지
	// PropmtText: "F: 채집하기" 같은 안내 문구
	void UpdateInteractionPrompt(bool bHasTarget, const FText& PromptText);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextBlock_InteractionPrompt;
};
