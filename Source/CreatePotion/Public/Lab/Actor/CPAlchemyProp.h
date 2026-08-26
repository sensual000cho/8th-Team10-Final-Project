 #pragma once

#include "CoreMinimal.h"
#include "Lab/Actor/CPThrowablePropBase.h"
#include "Lab/CPLabPotionRequestTypes.h"
#include "CPAlchemyProp.generated.h"

class UCPForageableItemData;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class CREATEPOTION_API ACPAlchemyProp
	: public ACPThrowablePropBase
{
	GENERATED_BODY()

public:
	ACPAlchemyProp();

	virtual void Tick(float DeltaSeconds) override;

	// 재료 전용 상호작용
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() override;
	virtual FName GetInteractionName_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Lab|Ingredient")
	void InitializeFromItemData(UCPForageableItemData* ItemData);

	void InitializeAlchemyProp(UCPForageableItemData* ItemData, const TArray<FGameplayTag>& EffectTags = TArray<FGameplayTag>());

	UFUNCTION(BlueprintPure, Category = "Lab|Ingredient")
	FCPLabIngredientInstance GetWorkingIngredient() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lab|Ingredient|Presentation")
	TObjectPtr<USceneComponent> IngredientUprightPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lab|Ingredient|Presentation")
	TObjectPtr<USceneComponent> IngredientBobblePivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lab|Ingredient|Presentation")
	TObjectPtr<UStaticMeshComponent> IngredientMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|Ingredient|Presentation")
	bool bEnableIngredientBobble = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|Ingredient|Presentation", meta = (ClampMin = "0.0", Units = "cm"))
	float BobbleAmplitude = 2.5f;

	// 초당 부유 왕복 횟수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|Ingredient|Presentation", meta = (ClampMin = "0.0", Units = "Hz"))
	float BobbleSpeed = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|Ingredient|Presentation", meta = (ClampMin = "0.0"))
	float UprightRecoverySpeed = 0.9f;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Ingredient")
	FCPLabIngredientInstance WorkingIngredient;

	FVector IngredientBobbleBaseLocation = FVector::ZeroVector;
};
