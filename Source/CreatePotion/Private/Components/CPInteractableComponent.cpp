// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CPInteractableComponent.h"

#include "Character/CPInteractionComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/Widgets/Common/Interaction/CPInteractionPromptWidget.h"

UCPInteractableComponent::UCPInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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
			InteractWidgetComp->SetVisibility(false);
		}
	}
	
	// Interaction Component쪽 델리게이트 바인딩
	// InteractionComp->OnInteractionStarted.AddUniqueDynamic(this, &UCPInteractableComponent::HandleInteractionStarted);
}

void UCPInteractableComponent::BeginFocus(AActor* Interactor)
{
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
	
}

void UCPInteractableComponent::EndFocus(AActor* Interactor)
{
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





