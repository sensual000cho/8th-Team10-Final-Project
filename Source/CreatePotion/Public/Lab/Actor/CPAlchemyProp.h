#pragma once

#include "CoreMinimal.h"
#include "Lab/Actor/CPThrowablePropBase.h"
#include "Lab/CPLabPotionRequestTypes.h"
#include "CPAlchemyProp.generated.h"

class UCPForageableItemData;

UCLASS()
class CREATEPOTION_API ACPAlchemyProp
	: public ACPThrowablePropBase
{
	GENERATED_BODY()

public:
	// 재료 전용 상호작용
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Lab|Ingredient")
	void InitializeFromItemData(UCPForageableItemData* ItemData);

	void InitializeAlchemyProp(UCPForageableItemData* ItemData, const TArray<FGameplayTag>& EffectTags = TArray<FGameplayTag>());

	UFUNCTION(BlueprintPure, Category = "Lab|Ingredient")
	FCPLabIngredientInstance GetWorkingIngredient() const;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Ingredient")
	FCPLabIngredientInstance WorkingIngredient;
};