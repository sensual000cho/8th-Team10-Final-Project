// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/CPBaseInteractableActor.h"

#include "Components/CPInteractableComponent.h"
#include "Character/CPInteractionComponent.h"
#include "Components/WidgetComponent.h"
#include "Settings/CPUISettings.h"


// Sets default values
ACPBaseInteractableActor::ACPBaseInteractableActor()
{
	InteractableComponent = CreateDefaultSubobject<UCPInteractableComponent>("InteractableComponent");
	InteractionWidget = CreateDefaultSubobject<UWidgetComponent>("InteractionWidget");
	
	if (InteractableComponent)
	{
		InteractableComponent->SetInteractionWidgetComponent(InteractionWidget);
	}
	
	// 상호작용 위젯 클래스 설정
	if (const UCPUISettings* UISettings = GetDefault<UCPUISettings>())
	{
		InteractionWidget->SetWidgetClass(UISettings->InteractionWidgetClass.Get());
	}
}

void ACPBaseInteractableActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (InteractionWidget && RootComponent && !InteractionWidget->GetAttachParent())
	{
		InteractionWidget->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void ACPBaseInteractableActor::CheckInteractionConditions()
{
	if (!IsValid(InteractableComponent)) return;
	
	SetDisplayState(EvaluateDisplayState());
	SetInteractability(EvaluateInteractability());
	SetInteractionPrompt(EvaluateInteractionPrompt());
	SetTargetName(EvaluateTargetName());
	SetInteractionDuration(EvaluateInteractionDuration());
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
	CheckInteractionConditions();
}

void ACPBaseInteractableActor::HandleInteractionStarted(AActor* Interactor)
{
	CheckInteractionConditions();
}

bool ACPBaseInteractableActor::EvaluateInteractability_Implementation() const
{
	return IsValid(InteractableComponent) ? InteractableComponent->CanInteract() : false;
}

ECPInteractionDisplayState ACPBaseInteractableActor::EvaluateDisplayState_Implementation() const
{
	return IsValid(InteractableComponent) ? InteractableComponent->GetDisplayState() : ECPInteractionDisplayState::Hidden;
}

FName ACPBaseInteractableActor::EvaluateTargetName_Implementation() const
{
	return IsValid(InteractableComponent) ? InteractableComponent->GetTargetName() : NAME_None;
}

FText ACPBaseInteractableActor::EvaluateInteractionPrompt_Implementation() const
{
	return IsValid(InteractableComponent) ? InteractableComponent->GetInteractionPrompt() : FText::GetEmpty();
}

float ACPBaseInteractableActor::EvaluateInteractionDuration_Implementation() const
{
	return IsValid(InteractableComponent) ? InteractableComponent->GetInteractionDuration() : 0.0f;
}

void ACPBaseInteractableActor::SetDisplayState(ECPInteractionDisplayState NewDisplayState)
{
	if (!IsValid(InteractableComponent)) return;
	
	InteractableComponent->SetDisplayState(NewDisplayState);
}

void ACPBaseInteractableActor::SetInteractability(bool bValue)
{
	if (!IsValid(InteractableComponent)) return;
	
	InteractableComponent->SetInteractability(bValue);
}

void ACPBaseInteractableActor::SetInteractionPrompt(FText NewPrompt)
{
	if (!IsValid(InteractableComponent)) return;
	
	InteractableComponent->SetInteractionPrompt(NewPrompt);
}

void ACPBaseInteractableActor::SetTargetName(FName NewName)
{
	if (!IsValid(InteractableComponent)) return;
	
	InteractableComponent->SetTargetName(NewName);
}

void ACPBaseInteractableActor::SetInteractionDuration(float NewDuration)
{
	if (!IsValid(InteractableComponent)) return;
	
	InteractableComponent->SetInteractionDuration(NewDuration);
}

bool ACPBaseInteractableActor::CanInteract() const
{
	return IsValid(InteractableComponent) ? InteractableComponent->CanInteract() : false;
}

ECPInteractionDisplayState ACPBaseInteractableActor::GetDisplayState() const
{
	return IsValid(InteractableComponent) ? InteractableComponent->GetDisplayState() : ECPInteractionDisplayState::Hidden;
}

FName ACPBaseInteractableActor::GetTargetName() const
{
	return IsValid(InteractableComponent) ? InteractableComponent->GetTargetName() : NAME_None;
}

FText ACPBaseInteractableActor::GetInteractionPrompt() const
{
	return IsValid(InteractableComponent) ? InteractableComponent->GetInteractionPrompt() : FText::GetEmpty();
}

float ACPBaseInteractableActor::GetInteractionDuration() const
{
	return IsValid(InteractableComponent) ? InteractableComponent->GetInteractionDuration() : 0.0f;
}





