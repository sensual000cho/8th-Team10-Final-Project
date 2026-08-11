// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Lab/Component/CPLabInteractActionComponent.h"
#include "CPLabPotionFinalizeComponent.generated.h"

class UCPForageableItemData;
class ACPAlchemyProp;
/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPLabPotionFinalizeComponent : public UCPLabInteractActionComponent
{
	GENERATED_BODY()
	
public:
	virtual bool ExecuteInteraction(AActor* Interactor) override;
	virtual bool CanExecuteInteraction(AActor* Interactor) const override;
};
