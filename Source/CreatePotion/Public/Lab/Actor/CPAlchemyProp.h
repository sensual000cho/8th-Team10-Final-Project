#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Lab/CPLabPotionRequestTypes.h"
#include "CPAlchemyProp.generated.h"

class UStaticMeshComponent;
class UCPForageableItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCPOnAlchemyPropChanged);

// 공방에서 들고 다니며 가공하는 물리 재료 Actor
UCLASS()
class CREATEPOTION_API ACPAlchemyProp : public AActor
{
	GENERATED_BODY()
	
public:	
	ACPAlchemyProp();
	
	// Prop의 작업 재료값이 바뀌었음을 알림
	UPROPERTY(BlueprintAssignable, Category = "Lab|Ingredient")
	FCPOnAlchemyPropChanged OnAlchemyPropChanged;
	
	// ItemData의 원본 효과값을 복사해 새 작업 재료로 초기화
	UFUNCTION(BlueprintCallable, Category = "Lab|Ingredient")
	void InitializeFromItemData(UCPForageableItemData* ItemData);
	
	UFUNCTION(BlueprintPure, Category = "Lab|Ingredient")
	FCPLabIngredientInstance GetWorkingIngredient() const;
	
	UFUNCTION(BlueprintCallable, Category = "Lab|Ingredient")
	bool SetWorkingIngredient(const FCPLabIngredientInstance& Ingredient);

	UFUNCTION(BlueprintPure, Category = "Lab|Ingredient")
	UCPForageableItemData* GetSourceItemData() const;

	UFUNCTION(BlueprintPure, Category = "Lab|Ingredient")
	int32 GetEffectValue(const FGameplayTag& EffectTag) const;

private:
	// 현재 작업 재료 정보를 비움
	void ResetWorkingIngredient();

private:
	UPROPERTY(VisibleAnywhere, Category = "Lab|Ingredient")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Ingredient")
	FCPLabIngredientInstance WorkingIngredient;
};
