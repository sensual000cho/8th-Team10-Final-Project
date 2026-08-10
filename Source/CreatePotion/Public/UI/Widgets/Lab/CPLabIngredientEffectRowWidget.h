// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPLabIngredientEffectRowWidget.generated.h"

class UTextBlock;
class UWidget;

UENUM(BlueprintType)
enum class ECPLabIngredientEffectDeltaState : uint8
{
	None,
	Increased,
	Decreased,
	Unchanged
};

/**
 * Displays one ingredient effect as independently styleable WBP blocks.
 * The WBP owns presentation such as colors, backgrounds, and the arrow image.
 */
UCLASS()
class CREATEPOTION_API UCPLabIngredientEffectRowWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void SetEffectData(
		const FText& InEffectName,
		int32 InCurrentLevel,
		bool bInHasPreview,
		int32 InPreviewLevel);

	UFUNCTION(BlueprintPure, Category = "Lab|UI|Ingredient")
	ECPLabIngredientEffectDeltaState GetDeltaState() const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Lab|UI|Ingredient", meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_EffectName;

	UPROPERTY(BlueprintReadOnly, Category = "Lab|UI|Ingredient", meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_CurrentLevel;

	// May be a Horizontal Box, Overlay, Border, or another WBP layout widget.
	UPROPERTY(BlueprintReadOnly, Category = "Lab|UI|Ingredient", meta = (BindWidget))
	TObjectPtr<UWidget> PreviewGroup;

	UPROPERTY(BlueprintReadOnly, Category = "Lab|UI|Ingredient", meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_PreviewLevel;

	/**
	 * Implement this in WBP_CP_LabIngredientEffectRow to style the arrow,
	 * preview level, background, and other presentation for each delta state.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lab|UI|Ingredient", meta = (DisplayName = "Apply Delta Style"))
	void BP_ApplyDeltaStyle(ECPLabIngredientEffectDeltaState InDeltaState);

private:
	static FText FormatLevel(int32 Level);

	UPROPERTY(Transient)
	ECPLabIngredientEffectDeltaState DeltaState = ECPLabIngredientEffectDeltaState::None;
};
