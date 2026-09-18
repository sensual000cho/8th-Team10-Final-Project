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
	UCPLabBellComponent();
	
	// Interact
	virtual bool TryInteract(AActor* Interactor) override;
	virtual bool CanInteract() const override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPQuestSelectWidget> QuestSelectWidgetClass;
};
