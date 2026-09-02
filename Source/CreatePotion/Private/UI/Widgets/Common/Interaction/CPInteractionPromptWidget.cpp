// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Interaction/CPInteractionPromptWidget.h"
#include "Character/CPInteractionComponent.h"
#include "Components/TextBlock.h"

// void UCPInteractionPromptWidget::BindEvents()
// {
// 	APawn* OwningPawn = GetOwningPlayerPawn();
// 	if (!OwningPawn) return;
// 	
// 	BoundInteractionComponent = OwningPawn->FindComponentByClass<UCPInteractionComponent>();
// 	if (BoundInteractionComponent)
// 	{
// 		BoundInteractionComponent->OnPromptChanged.AddDynamic(this, &UCPInteractionPromptWidget::OnPromptChanged);
// 	}
// 	
// 	
// 	//HideWidget();
// }
//
// void UCPInteractionPromptWidget::UnbindEvents()
// {
// 	// 구독한 델리게이트 해제 
// 	if (BoundInteractionComponent)
// 	{
// 		BoundInteractionComponent->OnPromptChanged.RemoveDynamic(this, &UCPInteractionPromptWidget::OnPromptChanged);
// 		BoundInteractionComponent = nullptr;
// 	}
// }
//
// void UCPInteractionPromptWidget::OnPromptChanged(FText Prompt, FName TargetName)
// {
// 	bool bHasTarget = !Prompt.IsEmpty();
// 	
// 	if (bHasTarget && TextBlock_InteractionPrompt)
// 	{
// 		TextBlock_ActorName->SetText(FText::FromName(TargetName));
// 		TextBlock_InteractionPrompt->SetText(Prompt);
// 	}
// 	
// 	bHasTarget ? ShowWidget() : HideWidget();
// }
void UCPInteractionPromptWidget::UpdateUI(const FText& Prompt, const FName& TargetName)
{
	if (IsValid(TextBlock_ActorName))
	{
		TextBlock_ActorName->SetText(FText::FromName(TargetName));
	}
	
	if (IsValid(TextBlock_InteractionPrompt))
	{
		TextBlock_InteractionPrompt->SetText(Prompt);
	}
}
