// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Interaction/CPInteractionPromptWidget.h"
#include "Character/CPInteractionComponent.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void UCPInteractionPromptWidget::BindEvents()
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn) return;
	
	BoundInteractionComponent = OwningPawn->FindComponentByClass<UCPInteractionComponent>();
	if (BoundInteractionComponent)
	{
		BoundInteractionComponent->OnPromptChanged.AddDynamic(this, &UCPInteractionPromptWidget::OnPromptChanged);
	}
	
	// WBP의 원래 색상 저장
	CacheDefaultColors();
	
	HideWidget();
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

void UCPInteractionPromptWidget::OnPromptChanged(FText Prompt, FName TargetName, ECPInteractionDisplayState DisplayState)
{
	const bool bHasTarget = !Prompt.IsEmpty() && DisplayState != ECPInteractionDisplayState::Hidden;

	if (!bHasTarget)
	{
		HideWidget();
		return;
	}

	if (TextBlock_ActorName)
	{
		TextBlock_ActorName->SetText(FText::FromName(TargetName));
	}

	if (TextBlock_InteractionPrompt)
	{
		TextBlock_InteractionPrompt->SetText(Prompt);
	}

	ApplyDisplayStateColors(DisplayState);
	ShowWidget();
}

void UCPInteractionPromptWidget::CacheDefaultColors()
{
	if (bDefaultColorsCached)
	{
		return;
	}

	if (!Border_InputKey || !TextBlock_InputKey || !TextBlock_ActorName || !TextBlock_InteractionPrompt)
	{
		return;
	}

	DefaultInputKeyBackgroundColor = Border_InputKey->GetBrushColor();
	DefaultInputKeyTextColor = TextBlock_InputKey->GetColorAndOpacity();
	DefaultActorNameTextColor = TextBlock_ActorName->GetColorAndOpacity();
	DefaultInteractionPromptTextColor = TextBlock_InteractionPrompt->GetColorAndOpacity();

	bDefaultColorsCached = true;
}

void UCPInteractionPromptWidget::ApplyDisplayStateColors(ECPInteractionDisplayState DisplayState)
{
	if (!bDefaultColorsCached)
	{
		CacheDefaultColors();
	}

	if (!bDefaultColorsCached)
	{
		return;
	}

	const bool bUnavailable = DisplayState == ECPInteractionDisplayState::Disabled;

	if (bUnavailable)
	{
		const FLinearColor UnavailableInputKeyBackground = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("000000FF")));
		
		const FLinearColor UnavailableTextColor = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("474747FF")));

		Border_InputKey->SetBrushColor(UnavailableInputKeyBackground);
		TextBlock_InputKey->SetColorAndOpacity(FSlateColor(UnavailableTextColor));
		TextBlock_ActorName->SetColorAndOpacity(FSlateColor(UnavailableTextColor));
		TextBlock_InteractionPrompt->SetColorAndOpacity(FSlateColor(UnavailableTextColor));

		return;
	}

	// Enabled 상태가 되면 WBP에서 설정했던 원래 색상으로 복구
	Border_InputKey->SetBrushColor(DefaultInputKeyBackgroundColor);
	TextBlock_InputKey->SetColorAndOpacity(DefaultInputKeyTextColor);
	TextBlock_ActorName->SetColorAndOpacity(DefaultActorNameTextColor);
	TextBlock_InteractionPrompt->SetColorAndOpacity(DefaultInteractionPromptTextColor);
}
