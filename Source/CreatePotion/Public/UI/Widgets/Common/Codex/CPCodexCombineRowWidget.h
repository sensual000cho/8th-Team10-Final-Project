// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPCodexCombineRowWidget.generated.h"

class UBorder;
class UImage;
class UTextBlock;
class UCPForageableItemData;

struct CREATEPOTION_API FCPTagCodexDisplayData
{
	FGameplayTag Tag;
	FText Text;
	TOptional<FLinearColor> Color;
};

UCLASS()
class CREATEPOTION_API UCPCodexCombineRowWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()
	
public:
	void SetCombinationData(
		const FCPTagCodexDisplayData& LeftData, const FCPTagCodexDisplayData& RightData, const FCPTagCodexDisplayData& ResultData);
	
private:
	void ApplyIngredientImage(UImage* ImageWidget, UCPForageableItemData* ItemData);
	void ApplyDisplay(UBorder* Border, UTextBlock* TextBlock, const FCPTagCodexDisplayData& Data, bool bDiscovered);
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Codex|Tag", meta = (AllowPrivateAccess = "true"))
	FLinearColor UnDiscoveredTagColor;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Codex|Tag", meta = (AllowPrivateAccess = "true"))
	float TagColorOpacity;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_LeftImage;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_RightImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_Left;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_Right;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_Result;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_LeftTag;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_RightTag;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_ResultTag;
};
