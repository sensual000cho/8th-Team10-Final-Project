// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPLabPotionResultWidget.generated.h"

class UCPLabPotionSessionComponent;
class UCPLabIngredientEffectRowWidget;
class UTextBlock;
class UVerticalBox;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPLabPotionResultWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()
	
	
protected:
	virtual void NativeConstruct() override;
	virtual void BindEvents() override;
	virtual void UnbindEvents() override;
	
private:
	UFUNCTION()
	void HandlePotionResultChanged(const TArray<FGameplayTag>& EffectTotals);
	
	UFUNCTION()
	void HandleSessionChanged();
	
	void ReBuildEffectRows(const TArray<FGameplayTag>& EffectTotals);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_NoResult;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_EffectRows;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|UI|Result")
	TSubclassOf<UCPLabIngredientEffectRowWidget> EffectRowWidgetClass;
	
private:
	UPROPERTY(Transient)
	TObjectPtr<UCPLabPotionSessionComponent> BoundPotionSession;
};
