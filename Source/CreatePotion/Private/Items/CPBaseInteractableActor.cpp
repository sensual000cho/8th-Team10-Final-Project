// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/CPBaseInteractableActor.h"

#include "Components/CPInteractableComponent.h"
#include "Components/WidgetComponent.h"


// Sets default values
ACPBaseInteractableActor::ACPBaseInteractableActor()
{
	TestInteractableComponent = CreateDefaultSubobject<UCPInteractableComponent>("InteractableComponent");
	TestInteractionWidget = CreateDefaultSubobject<UWidgetComponent>("InteractionWidget");
}

void ACPBaseInteractableActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (TestInteractionWidget && RootComponent && !TestInteractionWidget->GetAttachParent())
	{
		TestInteractionWidget->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

// Called when the game starts or when spawned
void ACPBaseInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (TestInteractableComponent)
	{
		TestInteractableComponent->OnActorInteractionStarted.AddUniqueDynamic(this, &ACPBaseInteractableActor::HandleInteractionStarted);
		TestInteractableComponent->OnActorInteractionCompleted.AddUniqueDynamic(this, &ACPBaseInteractableActor::HandleInteract);
	}
}

void ACPBaseInteractableActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TestInteractableComponent)
	{
		TestInteractableComponent->OnActorInteractionStarted.RemoveDynamic(this, &ACPBaseInteractableActor::HandleInteractionStarted);
		TestInteractableComponent->OnActorInteractionCompleted.RemoveDynamic(this, &ACPBaseInteractableActor::HandleInteract);
	}
	
	Super::EndPlay(EndPlayReason);
}


void ACPBaseInteractableActor::HandleInteract(AActor* Interactor)
{
}

void ACPBaseInteractableActor::HandleInteractionStarted(AActor* Interactor)
{
}

