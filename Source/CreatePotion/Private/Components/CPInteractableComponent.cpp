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
			UE_LOG(LogTemp, Error, TEXT("컴포넌트는 있지만, 내부 위젯 객체(UserWidget)가 생성되지 않았습니다!"));
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

void UCPInteractableComponent::Interact(AActor* Interactor)
{
	if (!bCanInteract) return;
	
	OnInteracted.Broadcast(Interactor);
	
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
	
	// 상태가 바뀌었을 때 스스로 갱신
	if (IsValid(InteractWidgetComp))
	{
		if (UCPInteractionPromptWidget* InteractionUI = Cast<UCPInteractionPromptWidget>(InteractWidgetComp->GetUserWidgetObject()))
		{
			InteractionUI->UpdateUI(InteractionPrompt, TargetName, CachedDisplayState);
		}
	}
}


