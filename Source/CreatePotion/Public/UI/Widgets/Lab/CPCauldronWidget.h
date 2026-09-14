// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Common/Interaction/CPInteractionPromptWidget.h"
#include "CPCauldronWidget.generated.h"

class UVerticalBox;
struct FGameplayTag;
class UCPCauldronComponent;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPCauldronWidget : public UCPInteractionPromptWidget
{
	GENERATED_BODY()
	
public:
	void SetCauldronComponent(UCPCauldronComponent* InCauldronComponent);
	
protected:
	virtual void UnbindEvents() override;
	virtual void ApplyDisplayStateColors(ECPInteractionDisplayState DisplayState) override;
	
private:
	// 효과 갱신
	UFUNCTION()
	void RefreshEffectTags();
	
	// Tag Definition에 저장된 DisplayName으로 반환
	FText GetEffectDisplayName(const FGameplayTag& EffectTag) const;
	
protected:	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_CurrentEffects;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_EffectTags;
	
private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UCPCauldronComponent> CauldronComponent;
};
