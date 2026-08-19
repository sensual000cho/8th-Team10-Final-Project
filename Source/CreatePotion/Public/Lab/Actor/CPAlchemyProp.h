#pragma once

#include "CoreMinimal.h"
#include "GameCore/Interface/CPInteractable.h"
#include "GameFramework/Actor.h"
#include "Lab/CPLabPotionRequestTypes.h"
#include "CPAlchemyProp.generated.h"

class UStaticMeshComponent;
class UCPForageableItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCPOnAlchemyPropChanged);

// 공방에서 들고 다니며 가공하는 물리 재료 Actor
UCLASS()
class CREATEPOTION_API ACPAlchemyProp : public AActor, public ICPInteractable
{
	GENERATED_BODY()
	
public:	
	ACPAlchemyProp();
	
	// Interactable
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() override;
	virtual FName GetInteractionName_Implementation() override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	
public:
	// ItemData의 원본 효과값을 복사해 새 작업 재료로 초기화(제거 예정)
	UFUNCTION(BlueprintCallable, Category = "Lab|Ingredient")
	void InitializeFromItemData(UCPForageableItemData* ItemData);
	
	// Intialize 관련 해당 함수로 교체 예정
	// 재료 관련 호출 시 배열은 채우지 않아도 됨(BP 호출 미지원)
	void InitializeAlchemyProp(UCPForageableItemData* ItemData, const TArray<FGameplayTag>& EffectTags = TArray<FGameplayTag>());
	
	UFUNCTION(BlueprintPure, Category = "Lab|Ingredient")
	FCPLabIngredientInstance GetWorkingIngredient() const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Lab|Ingredient")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Ingredient")
	FCPLabIngredientInstance WorkingIngredient;
};
