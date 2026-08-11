// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Lab/Component/CPProcessorComponent.h"
#include "CPRestorerComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPRestorerComponent : public UCPProcessorComponent
{
	GENERATED_BODY()
	
protected:
	virtual bool CanProcess(const ACPAlchemyProp* ItemInstance) const override;
	virtual void ApplyProcess(ACPAlchemyProp* ItemInstance) override;
};
