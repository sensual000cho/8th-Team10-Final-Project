// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Lab/CPLabTypes.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPLabIngredientInfoWidget.generated.h"

class UCPForageableItemData;
class UCPLabIngredientEffectRowWidget;
class ACPAlchemyProp;
class UImage;
class UTextBlock;
class UVerticalBox;

/**
 * Shared contextual card for a carried ingredient or an ingredient in a slot.
 * Receives a material snapshot from its owner and does not search world actors or slots.
 */
UCLASS()
class CREATEPOTION_API UCPLabIngredientInfoWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void SetIngredientInfo(const FCPLabIngredientInstance& InIngredient);

	// Observe one world ingredient and refresh whenever its working data changes.
	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void SetObservedIngredient(ACPAlchemyProp* InIngredientProp);

	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void ClearObservedIngredient();

	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void SetPreviewEffects(const TMap<FGameplayTag, int32>& InPreviewEffects);

	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void ClearPreviewEffects();

	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void ClearIngredientInfo();

	UFUNCTION(BlueprintPure, Category = "Lab|UI|Ingredient")
	bool HasIngredientInfo() const;

protected:
	virtual void NativeConstruct() override;
	virtual void UnbindEvents() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Context;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_MaterialName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_MaterialIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_EffectRows;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|UI|Ingredient")
	TSubclassOf<UCPLabIngredientEffectRowWidget> EffectRowWidgetClass;

	// Presentation-only label overrides. Gameplay tags remain the source of identity.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|UI|Ingredient")
	TMap<FGameplayTag, FText> EffectDisplayNames;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|UI|Ingredient")
	FText EmptyIngredientText = NSLOCTEXT(
		"CPLabIngredientInfoWidget",
		"EmptyIngredient",
		"비어 있음");

private:
	void ApplyIngredientInfo(const FCPLabIngredientInstance& InIngredient);
	void RefreshObservedIngredient();
	void UnbindObservedIngredient();

	UFUNCTION()
	void HandleObservedIngredientChanged();

	void RefreshWidget();
	void RefreshEmptyState();
	void RebuildEffectRows();
	FText GetEffectDisplayName(const FGameplayTag& EffectTag) const;

	UPROPERTY(Transient)
	FCPLabIngredientInstance Ingredient;

	UPROPERTY(Transient)
	TMap<FGameplayTag, int32> PreviewEffects;


	UPROPERTY(Transient)
	TWeakObjectPtr<ACPAlchemyProp> ObservedIngredientProp;
};
