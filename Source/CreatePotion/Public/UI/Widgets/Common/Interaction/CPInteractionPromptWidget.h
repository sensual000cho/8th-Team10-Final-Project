// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseFixedWidget.h"
#include "CPInteractionPromptWidget.generated.h"

class UCPInteractionComponent;
class UTextBlock;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPInteractionPromptWidget : public UCPBaseFixedWidget
{
	GENERATED_BODY()
	
protected:
	virtual void BindEvents() override;
	virtual void UnbindEvents() override;
	
	UFUNCTION()
	void OnPromptChanged(FText Prompt, FName TargetName, ECPInteractionDisplayState DisplayState);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextBlock_InteractionPrompt;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextBlock_ActorName;
	
	UPROPERTY()
	UCPInteractionComponent* BoundInteractionComponent;
};
