// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CPInteractableComponent.h"

void UCPInteractableComponent::Interact(AActor* Interactor)
{
	if (bCanInteract)
	{
		OnInteracted.Broadcast(Interactor);
	}
}
