// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CPInteractableComponent.h"

#include "Camera/CameraComponent.h"
#include "Character/CPInteractionComponent.h"
#include "Components/WidgetComponent.h"
#include "Settings/CPUISettings.h"
#include "UI/Widgets/Common/Interaction/CPInteractionPromptWidget.h"

UCPInteractableComponent::UCPInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCPInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
	
	AActor* Owner = GetOwner();
	if (!IsValid(Owner)) return;
	
	InteractionWidgetComp = NewObject<UWidgetComponent>(Owner, TEXT("InteractionWidget"));
	if (!IsValid(InteractionWidgetComp)) return;
	
	Owner->AddInstanceComponent(InteractionWidgetComp);
	
	InteractionWidgetComp->SetupAttachment(this);
	InteractionWidgetComp->SetRelativeLocation(FVector::ZeroVector);
	InteractionWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);

	UClass* WidgetClass = InteractionWidgetClass.Get();
	if (!WidgetClass){
		if (const UCPUISettings* UISettings = GetDefault<UCPUISettings>()){
			WidgetClass = UISettings->InteractionWidgetClass.LoadSynchronous();
		}
	}
	InteractionWidgetComp->SetWidgetClass(WidgetClass);

	InteractionWidgetComp->SetVisibility(false);
	InteractionWidgetComp->SetHiddenInGame(true);
	InteractionWidgetComp->RegisterComponent();
}

void UCPInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InteractionWidgetComp){
		InteractionWidgetComp->DestroyComponent();
		InteractionWidgetComp = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

void UCPInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	UpdateWidgetLocation(FocusedInteractor.Get());
}

void UCPInteractableComponent::BeginFocus(AActor* Interactor)
{
	FocusedInteractor = Interactor;
	SetComponentTickEnabled(true);
	PrimaryComponentTick.SetTickFunctionEnable(true);
	UpdateWidgetLocation(Interactor);
	
	if (InteractionWidgetComp){
		InteractionWidgetComp->SetVisibility(true);
		InteractionWidgetComp->SetHiddenInGame(false);
		RefreshUI();
	}
}

void UCPInteractableComponent::EndFocus(AActor* Interactor)
{
	FocusedInteractor = nullptr;
	SetComponentTickEnabled(false);
	PrimaryComponentTick.SetTickFunctionEnable(false);
	
	if (InteractionWidgetComp){
		InteractionWidgetComp->SetVisibility(false);
	}
}

bool UCPInteractableComponent::CanInteract() const 
{
	return true;
}

bool UCPInteractableComponent::TryInteract(AActor* Interactor)
{
	// 상속 후 세부 구현
	if (!CanInteract()) return false;
	
	return true;
}

void UCPInteractableComponent::BeginTimeInteraction(AActor* Interactor)
{
	// 상속 후 세부 구현
}

void UCPInteractableComponent::RefreshUI()
{
	const ECPInteractionDisplayState DisplayState = CanInteract() ? ECPInteractionDisplayState::Enabled : ECPInteractionDisplayState::Disabled;
	
	if (IsValid(InteractionWidgetComp))
	{
		if (UCPInteractionPromptWidget* InteractionUI = Cast<UCPInteractionPromptWidget>(InteractionWidgetComp->GetUserWidgetObject()))
		{
			InteractionUI->UpdateUI(InteractionPrompt, TargetName, DisplayState);
		}
	}
}

void UCPInteractableComponent::UpdateWidgetLocation(AActor* Interactor)
{
	if (!IsValid(Interactor) || !IsValid(InteractionWidgetComp)) return;
	
	const UCameraComponent* Camera = Interactor->FindComponentByClass<UCameraComponent>();
	AActor* Owner = GetOwner();
	
	if (!IsValid(Camera) || !IsValid(Owner)) return;
	
	const FVector AnchorLocation = Owner->GetActorLocation();
	const FVector WidgetOffset = Owner->GetActorQuat().UnrotateVector(GetComponentLocation() - AnchorLocation);
	const FVector WidgetLocation = 
		AnchorLocation 
		+ Camera->GetForwardVector() * WidgetOffset.X 
		+ Camera->GetRightVector() * WidgetOffset.Y 
		+ Camera->GetUpVector() * WidgetOffset.Z;
	
	InteractionWidgetComp->SetWorldLocation(WidgetLocation);
}
