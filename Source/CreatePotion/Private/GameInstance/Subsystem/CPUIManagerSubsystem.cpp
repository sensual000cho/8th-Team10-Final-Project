// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"

void UCPUIManagerSubsystem::CloseWidget(UUserWidget* Widget)
{
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] CloseWidget 호출됨, Widget = %s, OpenWidgets.Num()=%d"),
			Widget ? *Widget->GetName() : TEXT("NULL"), OpenWidgets.Num());
	
	int32 Index = OpenWidgets.IndexOfByPredicate([Widget](const FPopupEntry& Entry)
	{
		return Entry.Widget == Widget;
	});
	
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] Found Index = %d"), Index);
	
	if (Index != INDEX_NONE)
	{
		OpenWidgets[Index].Widget->RemoveFromParent();
		OpenWidgets.RemoveAt(Index);
		UpdateInputMode();
	}
}

void UCPUIManagerSubsystem::BringWidgetToFront(UUserWidget* Widget)
{
	int32 Index = OpenWidgets.IndexOfByPredicate([Widget](const FPopupEntry& Entry)
	{
		return Entry.Widget == Widget;
	});
	
	if (Index != INDEX_NONE && Index != OpenWidgets.Num() - 1)
	{
		// 기존의 위젯 제거 후 다시 추가하여 Zorder 최상위로 올리기
		FPopupEntry EntryToMove = OpenWidgets[Index];
		OpenWidgets.RemoveAt(Index);
		
		OpenWidgets.Add(EntryToMove);
		
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
	
	// 열린 위젯이 없으면 기본 게임모드로 복구
	if (OpenWidgets.Num() == 0)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
		return;
	}
	
	const FPopupEntry& TopEntry = OpenWidgets.Last();
	UUserWidget* TopWidget = TopEntry.Widget;
	ECPInputMode CurrentMode = TopEntry.InputMode;
	
	switch (CurrentMode)
	{
		case ECPInputMode::GameOnly:
		{
				FInputModeGameOnly InputMode;
				PC->SetInputMode(InputMode);
				PC->bShowMouseCursor = false;
				break;
		}
		case ECPInputMode::UIOnly:
		{
			FInputModeUIOnly InputMode;
			if (TopWidget) InputMode.SetWidgetToFocus(TopWidget->TakeWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
			break;
		}
		case ECPInputMode::GameAndUI:
		{
			FInputModeGameAndUI InputMode;
			if (TopWidget) InputMode.SetWidgetToFocus(TopWidget->TakeWidget());
			InputMode.SetHideCursorDuringCapture(false);
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
			break;
		}
	}
}

UUserWidget* UCPUIManagerSubsystem::PushWidgetBP(TSubclassOf<UUserWidget> WidgetClass, ECPInputMode InputMode)
{
	if (!WidgetClass) return nullptr;
	
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetGameInstance(), WidgetClass);
	
	if (Widget)
	{
		Widget->AddToViewport(OpenWidgets.Num());
		
		
		// if (auto* PopupWidget = Cast<UCPBasePopupWidget>(Widget))
		// {
		// 	Mode = PopupWidget->GetInputMode();
		// 	
		// }
		
		OpenWidgets.Add({Widget, InputMode});
		UpdateInputMode();
	}
	return Widget;
}


