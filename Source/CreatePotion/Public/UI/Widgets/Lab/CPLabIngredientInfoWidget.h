// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Lab/CPLabTypes.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPLabIngredientInfoWidget.generated.h"

class UCPForageableItemData;
class UImage;
class UTextBlock;
class UVerticalBox;

//공방에서 운반 중이거나 슬롯에 놓인 재료 정보를 표시하는 공용 카드.
//실제 재료 상태는 외부에서 전달받으며 월드 Actor나 슬롯을 직접 탐색하지 않는다.
 
UCLASS()
class CREATEPOTION_API UCPLabIngredientInfoWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()

public:
	// 원본 재료와 현재 가공 상태를 한 번에 갱신한다.
	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void SetIngredientInfo(const FCPLabIngredientInstance& InIngredient, const FText& InContextText);

	// 가공기를 바라보는 동안 표시할 예상 효과값을 갱신한다.
	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void SetPreviewEffects(const TMap<FGameplayTag, int32>& InPreviewEffects);

	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void ClearPreviewEffects();

	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void ClearIngredientInfo();

	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void ShowIngredientInfo();

	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void HideIngredientInfo();

	UFUNCTION(BlueprintPure, Category = "Lab|UI|Ingredient")
	bool HasIngredientInfo() const;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Context;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_MaterialName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_MaterialIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_EffectSummary;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_DeltaArea;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_PreviewDeltaSummary;

private:
	void RefreshWidget();
	void RefreshPreview();

	FText BuildEffectSummary() const;
	FText BuildPreviewDeltaSummary() const;

	UPROPERTY(Transient)
	FCPLabIngredientInstance Ingredient;

	UPROPERTY(Transient)
	TMap<FGameplayTag, int32> PreviewEffects;

	UPROPERTY(Transient)
	FText ContextText;
};
