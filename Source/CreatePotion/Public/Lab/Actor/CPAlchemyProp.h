#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Lab/CPLabPotionRequestTypes.h"
#include "CPAlchemyProp.generated.h"

class UStaticMeshComponent;
class UCPForageableItemData;

// 공방에서 들고 다니며 가공하는 물리 재료 Actor
UCLASS()
class CREATEPOTION_API ACPAlchemyProp : public AActor
{
	GENERATED_BODY()
	
public:	
	ACPAlchemyProp();
	
	// ItemData의 원본 효과값을 복사해 새 작업 재료로 초기화
	UFUNCTION(BlueprintCallable, Category = "Lab|Ingredient")
	void InitializeFromItemData(UCPForageableItemData* ItemData);

	// 슬롯에서 꺼낸 재료와 원래 위치를 기억하도록 초기화
	UFUNCTION(BlueprintCallable, Category = "Lab|Ingredient")
	bool InitializeFromRequestSlot(
		FName InRequestId,
		int32 InSourceSlotIndex,
		const FCPLabIngredientInstance& Ingredient);

	UFUNCTION(BlueprintPure, Category = "Lab|Ingredient")
	FCPLabIngredientInstance GetWorkingIngredient() const;

	UFUNCTION(BlueprintPure, Category = "Lab|Ingredient")
	UCPForageableItemData* GetSourceItemData() const;

	UFUNCTION(BlueprintPure, Category = "Lab|Ingredient")
	int32 GetEffectValue(const FGameplayTag& EffectTag) const;

	UFUNCTION(BlueprintPure, Category = "Lab|Ingredient")
	FName GetSourceRequestId() const;

	UFUNCTION(BlueprintPure, Category = "Lab|Ingredient")
	int32 GetSourceSlotIndex() const;

	UFUNCTION(BlueprintPure, Category = "Lab|Ingredient")
	bool IsAssignedToRequestSlot() const;

private:
	// 현재 작업 재료와 슬롯 출처 정보를 모두 비움
	void ResetWorkingIngredient();

	UPROPERTY(VisibleAnywhere, Category = "Lab|Ingredient")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Ingredient")
	FCPLabIngredientInstance WorkingIngredient;

	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Ingredient")
	FName SourceRequestId = NAME_None;

	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Ingredient")
	int32 SourceSlotIndex = INDEX_NONE;
};
