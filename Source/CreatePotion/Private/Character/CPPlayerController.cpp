// CPPlayerController.cpp

#include "Character/CPPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "GameCore/Interface/CPLevelUIInterface.h"
#include "GameFramework/HUD.h"

#include "CreatePotion.h"   // 로그용 헤더
#include "UI/Widgets/Common/Container/CPContainerMainWidget.h"
#include "UI/Widgets/Common/Container/CPContainerGridWidget.h"
#include "Components/CPItemContainerComponent.h"
#include "UI/Context/CPContextInventoryOnly.h"

void ACPPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetContextHandlerForTargetContext(EUITargetContext::InventoryOnly);
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
void ACPPlayerController::SetContextHandlerForTargetContext(EUITargetContext InTargetContext)
{
	switch (InTargetContext)
	{
	case EUITargetContext::InventoryOnly:
		CurrentContextHandler = NewObject<UCPContextInventoryOnly>(this);
		break;
	case EUITargetContext::Lab:
		// TODO[Container] : UCPContextLab 구현 후 교체
		CurrentContextHandler = NewObject<UCPContextInventoryOnly>(this);
		break;
	case EUITargetContext::Storage:
		// TODO[Container] : UCPContextStorage 구현 후 교체
		CurrentContextHandler = NewObject<UCPContextInventoryOnly>(this);
		break;
	case EUITargetContext::Shop:
		// TODO[Container] : UCPContextShop 구현 후 교체
		CurrentContextHandler = NewObject<UCPContextInventoryOnly>(this);
		break;
	}
}

void ACPPlayerController::OpenExternalContainerUI(UCPItemContainerComponent* InTargetContainer)
{
	if (!InTargetContainer || !InTargetContainer->ContainerUIClass)
	{
		UE_LOG(LogContainer, Warning, TEXT("[Container] TargetContainer가 없거나 ContainerUIClass가 설정되지 않았습니다."));
		return;
	}

	// 이미 같은 컨테이너가 열려있다면 그대로 유지 (중복 생성 방지)
	if (CurrentContainerUIInstance && CurrentInteractingContainer == InTargetContainer)
	{
		return;
	}

	// 다른 컨테이너가 열려있었다면 먼저 닫고 새로 엶
	if (CurrentContainerUIInstance)
	{
		CloseExternalContainerUI();
	}

	CurrentContainerUIInstance = CreateWidget<UCPContainerMainWidget>(this, InTargetContainer->ContainerUIClass);
	if (!CurrentContainerUIInstance)
	{
		return;
	}

	CurrentContainerUIInstance->AddToViewport();
	CurrentContainerUIInstance->BindContainer(InTargetContainer);
	CurrentInteractingContainer = InTargetContainer;

	bShowMouseCursor = true;
	SetInputMode(FInputModeGameAndUI());

	SetContextHandlerForTargetContext(InTargetContainer->TargetContext);
}

void ACPPlayerController::CloseExternalContainerUI()
{
	if (CurrentContainerUIInstance)
	{
		CurrentContainerUIInstance->RemoveFromParent();
		CurrentContainerUIInstance = nullptr;
	}

	CurrentInteractingContainer = nullptr;

	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());

	// 컨테이너를 닫은 뒤에는 InventoryOnly로 변경
	SetContextHandlerForTargetContext(EUITargetContext::InventoryOnly);
}

void ACPPlayerController::ToggleExternalContainerUI(UCPItemContainerComponent* InTargetContainer)
{
	// 같은 컨테이너가 이미 열려있다면 닫기
	if (CurrentContainerUIInstance && CurrentInteractingContainer == InTargetContainer)
	{
		CloseExternalContainerUI();
		return;
	}

	OpenExternalContainerUI(InTargetContainer);
}
#pragma endregion