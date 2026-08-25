// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/CPLabGameMode.h"
#include "NPC/CPBaseNPC.h"
#include "CPLabNPC.generated.h"

class UCPLabResultWidget;

UCLASS()
class CREATEPOTION_API ACPLabNPC : public ACPBaseNPC
{
	GENERATED_BODY()
public:
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;

	UFUNCTION()
	void OpenResultWidget(AActor* Interactor);
	
	void HandleThrownPotionImpact(const TArray<FGameplayTag>& PotionEffectTags);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UCPLabResultWidget> ResultWidgetClass;
	
private:
	UPROPERTY(Transient)
	FCPPotionDeliveryResult PendingThrownPotionDeliveryResult;

	UPROPERTY(Transient)
	TObjectPtr<UCPLabResultWidget> ActiveResultWidget;

	UPROPERTY()
	class UCPNPCDialogueWidget* ActiveDialogueWidget;
};
