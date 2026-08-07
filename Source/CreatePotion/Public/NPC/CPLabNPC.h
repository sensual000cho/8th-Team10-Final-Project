// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/CPBaseNPC.h"
#include "CPLabNPC.generated.h"

/**
 * 
 */
UCLASS()
class CREATEPOTION_API ACPLabNPC : public ACPBaseNPC
{
	GENERATED_BODY()
public:
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() override;

};
