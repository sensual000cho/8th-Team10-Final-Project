// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/CPLabInteractActionComponent.h"

// Sets default values for this component's properties
UCPLabInteractActionComponent::UCPLabInteractActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bEnabled = true;
}

bool UCPLabInteractActionComponent::ExecuteInteraction(AActor* Interacter)
{
	return CanExecuteInteraction(Interacter);
}

bool UCPLabInteractActionComponent::CanExecuteInteraction(AActor* Interacter) const
{
	return bEnabled && Interacter != nullptr;
}

FText UCPLabInteractActionComponent::GetInteractionPrompt() const
{
	return InteractionPrompt;
}
