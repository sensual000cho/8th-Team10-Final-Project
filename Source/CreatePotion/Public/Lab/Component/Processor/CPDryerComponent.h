// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Lab/Component/CPProcessorComponent.h"
#include "CPDryerComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPDryerComponent : public UCPProcessorComponent
{
	GENERATED_BODY()
	
public:
	UCPDryerComponent();
	
protected:
	virtual bool CanProcess(const ACPAlchemyProp* ItemInstance) const override;
	virtual void ApplyProcess(ACPAlchemyProp* ItemInstance) override;
	virtual bool BuildPreviewEffects(
		const ACPAlchemyProp* ItemInstance,
		TMap<FGameplayTag, int32>& InOutPreviewEffects) const override;
	
private:
	static int32 CalculateDriedValue(int32 CurrentValue, int32 AppliedAmount);

	UPROPERTY(EditDefaultsOnly, Category = "Lab/Processor")
	int32 ProcessAmount;
	
};
