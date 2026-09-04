// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/CPBaseNPC.h"
#include "CPQuestNPC.generated.h"

class UCPQuestNPCDataAsset;

UCLASS()
class CREATEPOTION_API ACPQuestNPC : public ACPBaseNPC
{
	GENERATED_BODY()

public:
	ACPQuestNPC();

	virtual FText GetInteractionPrompt_Implementation() override;

	UFUNCTION(BlueprintPure, Category = "NPC|Data")
	const UCPQuestNPCDataAsset* GetQuestNPCData() const;

};