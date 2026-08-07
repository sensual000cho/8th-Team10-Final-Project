// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"

void UCPUIManagerSubsystem::CloseWidget(UUserWidget* Widget)
{
	int32 Index = OpenWidgets.IndexOfByPredicate([Widget](const FPopupEntry& Entry)
	{
		return Entry.Widget == Widget;
	});
	if (Index != INDEX_NONE)
	{
		OpenWidgets[Index].Widget->RemoveFromParent();
		OpenWidgets.RemoveAt(Index);
		UpdateInputMode();
	}
}

void UCPUIManagerSubsystem::CloseTopWidget()
{
	if (OpenWidgets.Num() == 0) return;
	CloseWidget(OpenWidgets.Last().Widget);
}

void UCPUIManagerSubsystem::UpdateInputMode()
{
	APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
	if (!PC) return;
	
	bool bNeedsUIFocus = false;
	UUserWidget* FocusTarget = nullptr;
	
	for (int32 i = OpenWidgets.Num() - 1; i >= 0; --i) 
	{
		if (OpenWidgets[i].bRequiresUIFocus)
		{
			bNeedsUIFocus = true;
			FocusTarget = OpenWidgets[i].Widget;
			break; 
		}
	}
	
	if (bNeedsUIFocus)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(FocusTarget->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
	else
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

UUserWidget* UCPUIManagerSubsystem::PushWidgetBP(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass) return nullptr;
	
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetGameInstance(), WidgetClass);
	
	if (Widget)
	{
		Widget->AddToViewport(OpenWidgets.Num());
		bool bFocus = true;
		if (auto* PopupWidget = Cast<UCPBasePopupWidget>(Widget))
		{
			bFocus = PopupWidget->RequiresUIFocus();
		}
		
		OpenWidgets.Add({Widget, bFocus});
		UpdateInputMode();
	}
	return Widget;
}


