// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CPInteractableComponent.h"

#include "Camera/CameraComponent.h"
#include "Character/CPInteractionComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/Widgets/Common/Interaction/CPInteractionPromptWidget.h"

UCPInteractableComponent::UCPInteractableComponent() : RelativeLocation(FVector::ZeroVector)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCPInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// WidgetComponent 캐싱
	if (AActor* Owner = GetOwner())
	{
		InteractWidgetComp = Owner->FindComponentByClass<UWidgetComponent>();
		
		if (InteractWidgetComp)
		{
			RelativeLocation = InteractWidgetComp->GetRelativeLocation();
			InteractWidgetComp->SetVisibility(false);
		}
	}
	
	// Interaction Component쪽 델리게이트 바인딩
	// InteractionComp->OnInteractionStarted.AddUniqueDynamic(this, &UCPInteractableComponent::HandleInteractionStarted);
}

void UCPInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	UpdateInteractionWidgetLocation(FocusedInteractor.Get());
}

void UCPInteractableComponent::BeginFocus(AActor* Interactor)
{
	FocusedInteractor = Interactor;
	SetComponentTickEnabled(true);
	PrimaryComponentTick.SetTickFunctionEnable(true);
	UpdateInteractionWidgetLocation(Interactor);
	
	if (InteractWidgetComp)
	{
		InteractWidgetComp->SetVisibility(true);
		InteractWidgetComp->SetHiddenInGame(false);
		
		if (UCPInteractionPromptWidget* InteractionUI = Cast<UCPInteractionPromptWidget>(InteractWidgetComp->GetUserWidgetObject()))
		{
			InteractionUI->UpdateUI(InteractionPrompt, TargetName, CachedDisplayState);
		}
		
		UE_LOG(LogTemp, Warning, TEXT("[Success] 위젯 켬! 위젯 이름: %s, 현재 보임 상태: %d"), 
			   *InteractWidgetComp->GetName(), InteractWidgetComp->IsVisible());
	
		if (InteractWidgetComp->GetUserWidgetObject() == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s]컴포넌트는 있지만, 내부 위젯 객체(UserWidget)가 생성되지 않았습니다!"), *GetName());
		}
		
	}
	
	UE_LOG(LogTemp, Warning, TEXT(
		"[InteractableComponent] BeginFocus CanEverTick=%d TickEnabled=%d Registered=%d Active=%d"
	),
		PrimaryComponentTick.bCanEverTick,
		IsComponentTickEnabled(),
		IsRegistered(),
		IsActive()
	);
	
}

void UCPInteractableComponent::EndFocus(AActor* Interactor)
{
	FocusedInteractor = nullptr;
	SetComponentTickEnabled(false);
	PrimaryComponentTick.SetTickFunctionEnable(false);
	
	if (InteractWidgetComp)
	{
		InteractWidgetComp->SetVisibility(false);
	}
	UE_LOG(LogTemp, Warning, TEXT("[InteractableComponent]위젯 꺼짐!"))
}

void UCPInteractableComponent::HandleInteract(AActor* Interactor)
{
	if (!bCanInteract) return;
	
	OnActorInteractionCompleted.Broadcast(Interactor);
	
}

void UCPInteractableComponent::HandleInteractionStarted(AActor* Interactor)
{
	OnActorInteractionStarted.Broadcast(Interactor);
}

void UCPInteractableComponent::RefreshUI()
{
	if (IsValid(InteractWidgetComp))
	{
		if (UCPInteractionPromptWidget* InteractionUI = Cast<UCPInteractionPromptWidget>(InteractWidgetComp->GetUserWidgetObject()))
		{
			InteractionUI->UpdateUI(InteractionPrompt, TargetName, CachedDisplayState);
		}
	}
}

void UCPInteractableComponent::SetDisplayState(ECPInteractionDisplayState NewDisplayState)
{
	if (CachedDisplayState == NewDisplayState) return;
	
	CachedDisplayState = NewDisplayState;
	
	RefreshUI();
}

void UCPInteractableComponent::SetInteractability(bool bValue)
{
	bCanInteract = bValue;
}

void UCPInteractableComponent::SetInteractionPrompt(FText NewPrompt)
{
	if (InteractionPrompt.IdenticalTo(NewPrompt)) return;
	
	InteractionPrompt = NewPrompt;
	
	RefreshUI();
}

void UCPInteractableComponent::SetTargetName(FName NewName)
{
	if (TargetName == NewName) return;
	
	TargetName = NewName;
	
	RefreshUI();
}

void UCPInteractableComponent::SetInteractionDuration(float NewDuration)
{
	if (NewDuration <= 0.0) return;
	
	InteractionDuration = NewDuration;
	
	RefreshUI();
}

void UCPInteractableComponent::SetInteractionWidgetComponent(UWidgetComponent* InWidgetComp)
{
	InteractWidgetComp = InWidgetComp;
}

void UCPInteractableComponent::UpdateInteractionWidgetLocation(AActor* Interactor)
{
	if (!IsValid(Interactor) || !IsValid(InteractWidgetComp)) return;
	
	const UCameraComponent* Camera = Interactor->FindComponentByClass<UCameraComponent>();
	AActor* Owner = GetOwner();
	
	if (!IsValid(Camera) || !IsValid(Owner)) return;
	
	const FVector AnchorLocation = Owner->GetActorLocation();
	const FVector WidgetLocation = 
		AnchorLocation 
		+ Camera->GetForwardVector() * RelativeLocation.X 
		+ Camera->GetRightVector() * RelativeLocation.Y 
		+ Camera->GetUpVector() * RelativeLocation.Z;
	
	InteractWidgetComp->SetWorldLocation(WidgetLocation);
}





