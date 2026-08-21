// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Lab/Component/CPLabInteractActionComponent.h"
#include "CPLabBellComponent.generated.h"

class UCPQuestSelectWidget;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPLabBellComponent : public UCPLabInteractActionComponent
{
	GENERATED_BODY()
	
public:
	UCPLabBellComponent();
	
	// Interact
	virtual bool ExecuteInteraction(AActor* Interactor) override;
	virtual bool CanExecuteInteraction(AActor* Interactor) const override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPQuestSelectWidget> QuestSelectWidgetClass;
};
