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
	virtual void ShowResultDialogue() override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void OnResultDialogueClosed();
};