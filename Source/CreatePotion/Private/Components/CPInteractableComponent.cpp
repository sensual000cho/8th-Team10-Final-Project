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
	
	if (InteractionWidgetComp) return;
	AActor* Owner = GetOwner();
	if (!IsValid(Owner)) return;
	
	InteractionWidgetComp = NewObject<UWidgetComponent>(Owner, TEXT("InteractionWidget"));
	if (!InteractionWidgetComp) return;
	
	InteractionWidgetComp->RegisterComponent();
	Owner->AddInstanceComponent(InteractionWidgetComp);
	
	InteractionWidgetComp->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	InteractionWidgetComp->SetRelativeLocation(FVector::ZeroVector);
	
	if (const UCPUISettings* UISettings = GetDefault<UCPUISettings>()){
		InteractionWidgetComp->SetWidgetClass(UISettings->InteractionWidgetClass.LoadSynchronous());
	}
	InteractionWidgetComp->SetVisibility(false);
	InteractionWidgetComp->SetHiddenInGame(true);
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
		
		UE_LOG(LogTemp, Warning, TEXT("[Success] 위젯 켬! 위젯 이름: %s, 현재 보임 상태: %d"), 
			   *InteractionWidgetComp->GetName(), InteractionWidgetComp->IsVisible());
	
		if (InteractionWidgetComp->GetUserWidgetObject() == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s]컴포넌트는 있지만, 내부 위젯 객체(UserWidget)가 생성되지 않았습니다!"), *GetName());
		}
	}
}

void UCPInteractableComponent::EndFocus(AActor* Interactor)
{
	FocusedInteractor = nullptr;
	SetComponentTickEnabled(false);
	PrimaryComponentTick.SetTickFunctionEnable(false);
	
	if (InteractionWidgetComp)
	{
		InteractionWidgetComp->SetVisibility(false);
	}
	UE_LOG(LogTemp, Warning, TEXT("[InteractableComponent]위젯 꺼짐!"))
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
	
	const FVector WidgetOffset = GetRelativeLocation();
	const FVector AnchorLocation = Owner->GetActorLocation();
	const FVector WidgetLocation = 
		AnchorLocation 
		+ Camera->GetForwardVector() * WidgetOffset.X 
		+ Camera->GetRightVector() * WidgetOffset.Y 
		+ Camera->GetUpVector() * WidgetOffset.Z;
	
	InteractionWidgetComp->SetWorldLocation(WidgetLocation);
}
