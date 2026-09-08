// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/CPBaseNPC.h"
#include "CPQuestNPC.generated.h"

class UCPNPCDialogueWidget;
class UCPLabResultWidget;
class UCPQuestNPCDataAsset;

UCLASS()
class CREATEPOTION_API ACPQuestNPC : public ACPBaseNPC
{
	GENERATED_BODY()

public:
	ACPQuestNPC();
	virtual FText GetInteractionPrompt_Implementation() override;
	
	void HandleThrownPotionImpact(const TArray<FGameplayTag>& PotionEffectTags);
	
	UFUNCTION()
	virtual void OpenResultWidget(AActor* Interactor);
	
	UFUNCTION()
	void ShowResultDialogue();

protected:
	virtual void BeginPlay() override;
	virtual const TArray<FName>& GetPotionQuestIDs() const;
	
	FName FindAcceptedQuestId() const;
	
	UFUNCTION()
	void OnPotionReactionChanged(const FGameplayTag Tag, int32 NewCount);
protected:
	UPROPERTY(Transient)
	TObjectPtr<const UCPQuestNPCDataAsset> QuestData;
	
	UPROPERTY(Transient)
	TObjectPtr<UCPLabResultWidget> ActiveResultWidget;
	
	UPROPERTY()
	TObjectPtr<UCPNPCDialogueWidget> ActiveDialogueWidget;
	
	FName CurrentQuestId;
};