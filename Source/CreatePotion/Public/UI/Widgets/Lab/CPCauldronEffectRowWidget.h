// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CPCauldronEffectRowWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPCauldronEffectRowWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetEffectData(const FText& EffectName, const FLinearColor& EffectColor);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_EffectName;
};
