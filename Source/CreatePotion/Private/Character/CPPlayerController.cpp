// CPPlayerController.cpp


#include "Character/CPPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "GameCore/Interface/CPLevelUIInterface.h"
#include "GameFramework/HUD.h"

void ACPPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACPPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

void ACPPlayerController::OnQuestTogglePressed()
{
	AHUD* CurrentHUD = GetHUD();
	
	if (CurrentHUD)
	{
		if (CurrentHUD->Implements<UCPLevelUIInterface>())
		{
			UE_LOG(LogTemp, Warning, TEXT("[PC] HUD 인터페이스 확인 완료"));
			
			ICPLevelUIInterface::Execute_TogglePopup(CurrentHUD, QuestToggleTag);
		}
	}
	else
	{
		{
			UE_LOG(LogTemp, Error, TEXT("[PC] 현재 레벨에 HUD가 없습니다."));
		}
	}
}



