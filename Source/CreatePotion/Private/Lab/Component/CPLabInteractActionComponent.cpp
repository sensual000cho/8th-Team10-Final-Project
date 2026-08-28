// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/CPLabInteractActionComponent.h"

// Sets default values for this component's properties
UCPLabInteractActionComponent::UCPLabInteractActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bEnabled = true;
	bShowWhenUnavailable = false;
}

bool UCPLabInteractActionComponent::ExecuteInteraction(AActor* Interactor)
{
	return CanExecuteInteraction(Interactor);
}

bool UCPLabInteractActionComponent::CanExecuteInteraction(AActor* Interactor) const
{
	return bEnabled && Interactor != nullptr;
}

FText UCPLabInteractActionComponent::GetInteractionPrompt() const
{
	return InteractionPrompt;
}

FName UCPLabInteractActionComponent::GetInteractionName() const
{
	return InteractionName;
}

bool UCPLabInteractActionComponent::ShouldShowUnavailableInteraction(AActor* Interactor) const
{
	return bShowWhenUnavailable;
}