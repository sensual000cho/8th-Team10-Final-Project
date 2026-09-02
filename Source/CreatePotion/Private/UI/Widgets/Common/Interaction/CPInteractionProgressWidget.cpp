// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/Common/Interaction/CPInteractionProgressWidget.h"
#include "Character/CPInteractionComponent.h"
#include "Components/ProgressBar.h"

void UCPInteractionProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ResetProgress();
	HideWidget();
}


void UCPInteractionProgressWidget::SetProgress(float InProgress)
{
	if (!ProgressBar) return;
	
	const float ClampedProgress = FMath::Clamp(InProgress, 0.0f, 1.0f);
	
	ProgressBar->SetPercent(ClampedProgress);
}

void UCPInteractionProgressWidget::ResetProgress()
{
	SetProgress(0.f);
}

void UCPInteractionProgressWidget::BindEvents()
{
	Super::BindEvents();
	
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn) return;
	
	InteractionComponent = OwningPawn->GetComponentByClass<UCPInteractionComponent>();
	if (!InteractionComponent) return;
	
	InteractionComponent->OnInteractionProgressChanged.AddDynamic(this, &ThisClass::SetProgress);
	InteractionComponent->OnInteractionStarted.AddDynamic(this, &ThisClass::ShowWidget);
	InteractionComponent->OnInteractionCompleted.AddDynamic(this, &ThisClass::HandleInteractionEnded);
	InteractionComponent->OnInteractionCancelled.AddDynamic(this, &ThisClass::HandleInteractionEnded);
}

void UCPInteractionProgressWidget::UnbindEvents()
{
	Super::UnbindEvents();
	
	if (!InteractionComponent) return;
	
	InteractionComponent->OnInteractionProgressChanged.RemoveAll(this);
	InteractionComponent->OnInteractionStarted.RemoveAll(this);
	InteractionComponent->OnInteractionCompleted.RemoveAll(this);
	InteractionComponent->OnInteractionCancelled.RemoveAll(this);
	InteractionComponent = nullptr;
}

void UCPInteractionProgressWidget::HandleInteractionEnded()
{
	ResetProgress();
	HideWidget();
}
