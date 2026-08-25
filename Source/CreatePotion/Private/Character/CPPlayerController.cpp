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
#include "Components/CPHandItemContainerComponent.h"
#include "Components/CPInventoryComponent.h"
#include "UI/Context/CPContextInventoryOnly.h"

void ACPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetContextHandlerForTargetContext(EUITargetContext::Inventory);
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

// TODO[Container] : 추후 멀티플레이 확장시 Dedicated Server 환경에서 Client들은 실행이 되지 않음
void ACPPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (InPawn)
	{
		// 폰에 성공적으로 빙의되었을 때, 딱 한 번만 찾아서 캐싱
		CachedInventoryComponent = InPawn->FindComponentByClass<UCPInventoryComponent>();
		LeftClickPickedContainer = InPawn->FindComponentByClass<UCPHandItemContainerComponent>();

		if (CachedInventoryComponent)
		{
			UE_LOG(LogContainer, Log, TEXT("인벤토리 컴포넌트 캐싱 완료"));
		}

		if (LeftClickPickedContainer)
		{
			UE_LOG(LogContainer, Log, TEXT("아이템 집기 컴포넌트 캐싱 완료"));
		}
	}
	else
	{
		// 캐릭터가 사망하거나 빙의가 해제되면 nullptr로 안전하게 초기화
		CachedInventoryComponent = nullptr;
		LeftClickPickedContainer = nullptr;
	}
}

void ACPPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

#pragma region Container


void ACPPlayerController::SetContextHandlerForTargetContext(EUITargetContext InTargetContext)
{
	switch (InTargetContext)
	{
	case EUITargetContext::Inventory:
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

	// 컨테이너를 닫은 뒤에는 Inventory로 변경
	SetContextHandlerForTargetContext(EUITargetContext::Inventory);
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

bool ACPPlayerController::IsHoldingItem() const
{
	return LeftClickPickedContainer && LeftClickPickedContainer->ContainerItems.Num() > 0;;
}

void ACPPlayerController::ResetHoldingItem()
{
	if (LeftClickPickedContainer)
	{
		LeftClickPickedContainer->ContainerItems.Empty();
		LeftClickPickedContainer->OnContainerUpdated.Broadcast();
	}
	LeftClickPickedItemOriginContainer = nullptr;
	LeftClickPickedOriginSlotIndex = -1;
}
#pragma endregion