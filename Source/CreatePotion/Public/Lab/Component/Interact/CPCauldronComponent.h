// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Lab/CPLabTypes.h"
#include "Lab/Component/CPLabInteractActionComponent.h"
#include "CPCauldronComponent.generated.h"

class UCPLabPotionSessionComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPCauldronComponent : public UCPLabInteractActionComponent
{
	GENERATED_BODY()

public:
	UCPCauldronComponent();
	
	// Life Cycle
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// 상호작용
	virtual bool ExecuteInteraction(AActor* Interactor) override;
	virtual bool CanExecuteInteraction(AActor* Interactor) const override;
	
	// Getter
	UFUNCTION(BlueprintPure, Category = "Lab|Interaction")
	TArray<FGameplayTag> GetEffectTags() const;
	
private:
	// 슬롯(배열) 초기화, 리퀘스트 종료 시에만 작동
	UFUNCTION()
	void HandleSessionChanged();
	
	// UI 도입 or PR확인 이후 삭제
	UFUNCTION(BlueprintCallable, Category = "Lab|Interaction|Debug")
	void DebugPrintSlots() const;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Lab|Interaction")
	TArray<FCPLabIngredientInstance> IngredientInstances;
	
	UPROPERTY(EditAnywhere, Category = "Lab|Interaction")
	int32 MaxSlotCount;
	
	UPROPERTY()
	TObjectPtr<UCPLabPotionSessionComponent> BoundPotionSession;
};
