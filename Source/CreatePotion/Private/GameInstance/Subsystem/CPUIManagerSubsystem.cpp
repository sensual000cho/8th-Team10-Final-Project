// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "UI/Widgets/Common/Container/CPInventoryMainWidget.h"

UUserWidget* UCPUIManagerSubsystem::PushWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass) return nullptr;
	
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetGameInstance(), WidgetClass);
	
	if (Widget)
	{
		int32 ZOrder = 0;
		ECPInputMode InputMode = ECPInputMode::GameOnly;
		
		// 팝업위젯일 경우
		if (auto* PopupWidget = Cast<UCPBasePopupWidget>(Widget))
		{
			ZOrder = PopupWidget->ZOrder;
			InputMode = PopupWidget->GetInputMode();
			// UE_LOG(LogTemp, Warning, TEXT("[UIManager] PushWidgetBP InputMode 읽음: %d"), (int32)InputMode);
			OpenWidgets.Add({Widget, InputMode});
		}
		
		Widget->AddToViewport(ZOrder);
		UpdateInputMode();
	}
	return Widget;
}

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
		if (OpenWidgets[Index].Widget.IsValid())
		{
			OpenWidgets[Index].Widget->RemoveFromParent();
		}
		OpenWidgets.RemoveAt(Index);
		UpdateInputMode();
	}
}

void UCPUIManagerSubsystem::CloseTopWidget()
{
	if (OpenWidgets.Num() == 0) return;
	UUserWidget* Top = OpenWidgets.Last().Widget.Get();
	if (!IsValid(Top)) return;
}

UUserWidget* UCPUIManagerSubsystem::ToggleWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass) return nullptr;
	
	int32 FoundIndex = OpenWidgets.IndexOfByPredicate([WidgetClass](const FPopupEntry& Entry)
	{
		return Entry.Widget.Get() && Entry.Widget->GetClass() == WidgetClass;
	});
	
	if (FoundIndex != INDEX_NONE)
	{
		UUserWidget* TargetWidget = OpenWidgets[FoundIndex].Widget.Get();
		CloseWidget(TargetWidget);
		return nullptr;
	}
	else
	{
		{
			return PushWidget(WidgetClass);
		}
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

UUserWidget* UCPUIManagerSubsystem::FindOpenWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		return nullptr;
	}
	
	for (FPopupEntry PopupEntry : OpenWidgets)
	{
		if (PopupEntry.Widget.Get() && PopupEntry.Widget->IsA(WidgetClass))
		{
			return PopupEntry.Widget.Get();
		}
	}
	return nullptr;
}

void UCPUIManagerSubsystem::PlayWidgetSound(USoundBase* Sound)
{
	if (IsValid(Sound))
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Sound);
	}
}

void UCPUIManagerSubsystem::UpdateInputMode()
{
	
	
	APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
	if (!PC) return;
	
	auto Priority = [](ECPInputMode Mode)
	{
		switch (Mode)
		{
			case ECPInputMode::UIOnly:    return 2;
			case ECPInputMode::GameAndUI: return 1;
			default:					  return 0;
		}
	};
	
	ECPInputMode CurrentMode = ECPInputMode::GameOnly;
	UUserWidget* TopWidget = nullptr;
	
	for (int32 i = OpenWidgets.Num() - 1; i>= 0; --i)
	{
		const FPopupEntry& Entry = OpenWidgets[i];
		if (!Entry.Widget.IsValid()) continue;
		
		if (Priority(Entry.InputMode) > Priority(CurrentMode))
		{
			CurrentMode = Entry.InputMode;
			TopWidget = Entry.Widget.Get();
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[UIManager] -> 적용 모드 = %s, 포커스 = %s"),
		*UEnum::GetValueAsString(CurrentMode),
		*GetNameSafe(TopWidget));
	
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
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetHideCursorDuringCapture(false);
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
			break;
		}
	}
	
}

void UCPUIManagerSubsystem::RegisterCreatedWidget(UUserWidget* Widget)
{
	if (!IsValid(Widget)) return;
	
	if (!Widget->IsInViewport())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] %s가 뷰포트에 없는 상태로 등록됩니다."), *GetNameSafe(Widget));
	}
	
	const bool bAlreadyOpen = OpenWidgets.ContainsByPredicate(
		[Widget](const FPopupEntry& Entry){ return Entry.Widget == Widget;});
	if (bAlreadyOpen)
	{
		UpdateInputMode();
		return;
	}
			
	ECPInputMode InputMode = ECPInputMode::GameAndUI; // 디폴트값
	if (auto* PopupWidget = Cast<UCPBasePopupWidget>(Widget))
	{
		InputMode = PopupWidget->InputMode;
	}
			
	// 열린 위젯 배열에 추가
	OpenWidgets.Add({Widget, InputMode});
	UpdateInputMode();
}

void UCPUIManagerSubsystem::UnregisterCreatedWidget(UUserWidget* Widget)
{
	if (!IsValid(Widget)) return;
	
	const int32 Removed = OpenWidgets.RemoveAll(
		[Widget](const FPopupEntry& Entry){ return Entry.Widget == Widget; });
	
	if (Removed > 0)
	{
		UpdateInputMode();
	}
}

bool UCPUIManagerSubsystem::HandleEscape()
{
	// null 위젯 처리
	OpenWidgets.RemoveAll([](const FPopupEntry& Entry)
	{
		return !Entry.Widget.IsValid();	
	});
	
	
	// 열려있는 위젯이 없으면 ESC로 메뉴 열기
	if (OpenWidgets.IsEmpty())
	{
		return false;
	}
	
	// 열려있는 위젯이 있으면 최상위 위젯부터 닫기
	if (UCPBaseUserWidget* Top = Cast<UCPBaseUserWidget>(OpenWidgets.Last().Widget.Get()))
	{
		Top->HandleEscape();
	}
	
	return true;
}


