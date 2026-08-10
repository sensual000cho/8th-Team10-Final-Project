// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Lab/Component/CPProcessorComponent.h"
#include "CPCrusherComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPCrusherComponent : public UCPProcessorComponent
{
	GENERATED_BODY()
	
public:
	UCPCrusherComponent();
	
	virtual void ResetProcessor() override;
	
protected:
	virtual bool CanProcess(const ACPAlchemyProp* ItemInstance) const override;
	virtual void ApplyProcess(ACPAlchemyProp* ItemInstance) override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Lab/Processor")
	float ProcessMultiplier;
	
	UPROPERTY(EditDefaultsOnly, Category = "Lab/Processor")
	int32 MaxUseCount;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Lab/Processor")
	int32 UsedCount;
};
