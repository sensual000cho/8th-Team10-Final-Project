// CPPlayerController.cpp

#include "Character/CPPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "GameCore/Interface/CPLevelUIInterface.h"
#include "GameFramework/HUD.h"
#include "EngineUtils.h"		// Iterator

#include "Lab/Actor/CPLabContainerActor.h"		// Lab에서 재료 선택 컨테이너 사용
#include "UI/Widgets/Common/Container/CPContainerMainWidget.h"
#include "UI/Widgets/Common/Container/CPContainerGridWidget.h"
#include "Components/CPItemContainerComponent.h"
#include "Components/CPLabContainerComponent.h"

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

#pragma region Container
void ACPPlayerController::ToggleLabUI()
{
	// 이미 열려있다면 닫기 (메모리에서 완전히 파괴)
	if (LabUIInstance)
	{
		LabUIInstance->RemoveFromParent();
		LabUIInstance = nullptr; // 완벽한 파괴를 위해 참조를 끊는 nullptr 처리

		CurrentInteractingContainer = nullptr;	// 현재 상호작용중인 컨테이너도 nullptr로 초기화

		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
		return;
	}
	
	UCPLabContainerComponent* FoundLabComp = nullptr;
	// TODO : 현재 Toggle시 매 번 레벨에서 모든 액터를 순회를 돌며 찾도록 되어있음
	for (TActorIterator<ACPLabContainerActor> It(GetWorld()); It; ++It)
	{
		FoundLabComp = (*It)->LabContainerComponent;
		break;
	}

	if (!FoundLabComp)
	{
		return;
	}
	
	if (LabUIClass)
	{
		LabUIInstance = CreateWidget<UCPContainerMainWidget>(this, LabUIClass);
		if (LabUIInstance)
		{
			LabUIInstance->AddToViewport();
			
			LabUIInstance->BindContainer(FoundLabComp);
			CurrentInteractingContainer = FoundLabComp;

			bShowMouseCursor = true;
			SetInputMode(FInputModeGameAndUI());
		}
	}
}
#pragma endregion


