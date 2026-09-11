// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/CPBaseNPC.h"
#include "CPQuestNPC.generated.h"

class UCPNPCDialogueWidget;
class UCPQuestNPCDataAsset;

UCLASS()
class CREATEPOTION_API ACPQuestNPC : public ACPBaseNPC
{
	GENERATED_BODY()

public:
	ACPQuestNPC();
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	
	void HandleThrownPotionImpact(const TArray<FGameplayTag>& PotionEffectTags);
	
	UFUNCTION()
	virtual void ShowResultDialogue();

protected:
	virtual void BeginPlay() override;
	virtual const TArray<FName>& GetPotionQuestIDs() const;
	
	FName FindAcceptedQuestId() const;
	void OnTraceFocusLost();

protected:
	UPROPERTY(Transient)
	TObjectPtr<const UCPQuestNPCDataAsset> QuestData;
	
	UPROPERTY()
	TObjectPtr<UCPNPCDialogueWidget> ActiveDialogueWidget;
	
	FName CurrentQuestId;

	// 플레이어에게 상호작용으로 접근할 때 멈추는 거리 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Interaction")
	float StopRadius = 80.0f;

	// 현재 대화/상호작용 진행 중 플래그
	UPROPERTY(BlueprintReadOnly, Category = "NPC|Interaction")
	bool bIsInteracting = false;

private:
	FTimerHandle TraceLostTimerHandle;
	bool bIsPlayerLooking = false;
};