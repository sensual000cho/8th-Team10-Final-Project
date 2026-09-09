// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/CPBaseInteractableActor.h"

#include "Components/CPInteractableComponent.h"
#include "Components/WidgetComponent.h"


// Sets default values
ACPBaseInteractableActor::ACPBaseInteractableActor()
{
	InteractableComponent = CreateDefaultSubobject<UCPInteractableComponent>("InteractableComponent");
	InteractionWidget = CreateDefaultSubobject<UWidgetComponent>("InteractionWidget");
}

void ACPBaseInteractableActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (InteractionWidget && RootComponent && !InteractionWidget->GetAttachParent())
	{
		InteractionWidget->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

// Called when the game starts or when spawned
void ACPBaseInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (InteractableComponent)
	{
		InteractableComponent->OnActorInteractionStarted.AddUniqueDynamic(this, &ACPBaseInteractableActor::HandleInteractionStarted);
		InteractableComponent->OnActorInteractionCompleted.AddUniqueDynamic(this, &ACPBaseInteractableActor::HandleInteract);
	}
}

void ACPBaseInteractableActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InteractableComponent)
	{
		InteractableComponent->OnActorInteractionStarted.RemoveDynamic(this, &ACPBaseInteractableActor::HandleInteractionStarted);
		InteractableComponent->OnActorInteractionCompleted.RemoveDynamic(this, &ACPBaseInteractableActor::HandleInteract);
	}
	
	Super::EndPlay(EndPlayReason);
}


void ACPBaseInteractableActor::HandleInteract(AActor* Interactor)
{
}

void ACPBaseInteractableActor::HandleInteractionStarted(AActor* Interactor)
{
}

