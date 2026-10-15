// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CPInteractableComponent.h"
#include "CPLabBellComponent.generated.h"

class UCPQuestSelectWidget;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPLabBellComponent : public UCPInteractableComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<class USoundBase> BellSound;


	UCPLabBellComponent();
	
	// Life Cycle
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// Interact
	virtual void BeginFocus(AActor* Interactor) override;
	virtual bool TryInteract(AActor* Interactor) override;
	
private:
	void HandlePotionRequestStateChanged(bool bActiveRequest);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPQuestSelectWidget> QuestSelectWidgetClass;
};
