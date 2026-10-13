// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/Common/Interaction/CPInteractionPromptWidget.h"
#include "Character/CPInteractionComponent.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UCPInteractionPromptWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// WBP의 원래 색상 저장
	CacheDefaultColors();
}

void UCPInteractionPromptWidget::CacheDefaultColors()
{
	if (bDefaultColorsCached)
	{
		return;
	}

	if (!Image_InputKey || !TextBlock_ActorName || !TextBlock_InteractionPrompt)
	{
		return;
	}
	
	DefaultInputKeyColor = Image_InputKey->GetColorAndOpacity();
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
		const FLinearColor UnavailableTextColor = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("8C8C8CFF")));
		const FLinearColor UnavailableInputKeyColor = FLinearColor(1.f, 1.f, 1.f, 0.3f);

		Image_InputKey->SetColorAndOpacity(UnavailableInputKeyColor);
		TextBlock_ActorName->SetColorAndOpacity(FSlateColor(UnavailableTextColor));
		TextBlock_InteractionPrompt->SetColorAndOpacity(FSlateColor(UnavailableTextColor));

		return;
	}

	// Enabled 상태가 되면 WBP에서 설정했던 원래 색상으로 복구
	//Border_InputKey->SetBrushColor(DefaultInputKeyBackgroundColor);
	Image_InputKey->SetColorAndOpacity(DefaultInputKeyColor);
	TextBlock_ActorName->SetColorAndOpacity(DefaultActorNameTextColor);
	TextBlock_InteractionPrompt->SetColorAndOpacity(DefaultInteractionPromptTextColor);
}

void UCPInteractionPromptWidget::UpdateUI(const FText& Prompt, const FName& TargetName, ECPInteractionDisplayState DisplayState)
{
	if (IsValid(TextBlock_ActorName))
	{
		if (TargetName.IsNone()){
			TextBlock_ActorName->SetText(FText::GetEmpty());	
		}else{
			TextBlock_ActorName->SetText(FText::FromName(TargetName));	
		}
	}
	
	if (IsValid(TextBlock_InteractionPrompt))
	{
		TextBlock_InteractionPrompt->SetText(Prompt);
	}
	
	ApplyDisplayStateColors(DisplayState);
}


