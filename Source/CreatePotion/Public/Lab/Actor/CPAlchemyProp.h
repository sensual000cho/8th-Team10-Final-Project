// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Lab/CPLabTypes.h"
#include "CPAlchemyProp.generated.h"

class UStaticMeshComponent;
class UCPForageableItemData;

UCLASS()
class CREATEPOTION_API ACPAlchemyProp : public AActor
{
	GENERATED_BODY()
	
public:	
	ACPAlchemyProp();
	
	UFUNCTION(BlueprintCallable, Category = "Lab|Ingredient")
	void InitializeFromItemData(UCPForageableItemData* ItemData);

	UFUNCTION(BlueprintPure, Category = "Lab|Ingredient")
	UCPForageableItemData* GetSourceItemData() const;

	UFUNCTION(BlueprintPure, Category = "Lab|Ingredient")
	int32 GetEffectValue(const FGameplayTag& EffectTag) const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Lab|Ingredient")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Ingredient")
	FCPLabIngredientInstance WorkingIngredient;
	
};
