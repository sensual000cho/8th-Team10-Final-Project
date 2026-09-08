// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/CPQuestNPC.h"
#include "CPLabNPC.generated.h"

class UCPNPCDialogueWidget;

UCLASS()
class CREATEPOTION_API ACPLabNPC : public ACPQuestNPC
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual void OpenResultWidget(AActor* Interactor) override;

protected:
	virtual const TArray<FName>& GetPotionQuestIDs() const override;
};
