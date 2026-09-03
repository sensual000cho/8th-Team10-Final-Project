// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseFixedWidget.h"
#include "CPInteractionPromptWidget.generated.h"

enum class ECPInteractionDisplayState : uint8;
class UCPInteractionComponent;
class UTextBlock;
class UBorder;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPInteractionPromptWidget : public UCPBaseFixedWidget
{
	GENERATED_BODY()
	
public:
	// UI 텍스트 설정
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateUI(const FText& Prompt, const FName& TargetName, ECPInteractionDisplayState DisplayState);

protected:
	void NativeConstruct() override;

protected:	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_InputKey;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_InputKey;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_ActorName;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_InteractionPrompt;
	
	UPROPERTY()
	UCPInteractionComponent* BoundInteractionComponent;
	
private:
	FLinearColor DefaultInputKeyBackgroundColor;
	FSlateColor DefaultInputKeyTextColor;
	FSlateColor DefaultActorNameTextColor;
	FSlateColor DefaultInteractionPromptTextColor;

	bool bDefaultColorsCached = false;
	
	void CacheDefaultColors();
	void ApplyDisplayStateColors(ECPInteractionDisplayState DisplayState);
};
