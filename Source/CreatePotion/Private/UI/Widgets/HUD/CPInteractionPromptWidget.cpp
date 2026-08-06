// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/HUD/CPInteractionPromptWidget.h"
#include "Character/CPInteractionComponent.h"
#include "Components/TextBlock.h"

void UCPInteractionPromptWidget::BindEvents()
{
	// TODO: 상호작용 탐색 컴포넌트 완성 후 델리게이트 구독
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn) return;
	
	BoundInteractionComponent = OwningPawn->FindComponentByClass<UCPInteractionComponent>();
	if (BoundInteractionComponent)
	{
		BoundInteractionComponent->OnPromptChanged.AddDynamic(this, &UCPInteractionPromptWidget::OnPromptChanged);
	}
	
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCPInteractionPromptWidget::UnbindEvents()
{
	// 구독한 델리게이트 해제 
	if (BoundInteractionComponent)
	{
		BoundInteractionComponent->OnPromptChanged.RemoveDynamic(this, &UCPInteractionPromptWidget::OnPromptChanged);
		BoundInteractionComponent = nullptr;
	}
}

void UCPInteractionPromptWidget::OnPromptChanged(FText Prompt)
{
	bool bHasTarget = !Prompt.IsEmpty();
	SetVisibility(bHasTarget ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	
	if (bHasTarget && TextBlock_InteractionPrompt)
	{
		TextBlock_InteractionPrompt->SetText(Prompt);
	}
}
