// CPPlayerController.cpp


#include "Character/CPPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"

#include "GameMode/CPLabGameMode.h"		// Lab에서 재료 선택 컨테이너 사용
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

void ACPPlayerController::ToggleLabUI()
{
	// 1. 이미 열려있다면 닫기 (메모리에서 완전히 파괴)
	if (LabUIInstance)
	{
		LabUIInstance->RemoveFromParent();
		LabUIInstance = nullptr; // 완벽한 파괴를 위해 nullptr 처리

		CurrentInteractingContainer = nullptr;

		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
		return;
	}

	// 2. GameMode 가져오기 및 캐싱된 데이터 확인
	ACPLabGameMode* GM = Cast<ACPLabGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->CachedLabContainer)
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode에 캐싱된 공방 컨테이너가 없습니다"));
		return;
	}

	// 3. UI 생성 및 데이터 바인딩
	if (LabUIClass)
	{
		LabUIInstance = CreateWidget<UCPContainerMainWidget>(this, LabUIClass);
		if (LabUIInstance)
		{
			LabUIInstance->AddToViewport();

			// GameMode가 쥐고 있던 캐싱 데이터를 현재 타겟으로 설정!
			CurrentInteractingContainer = GM->CachedLabContainer;

			LabUIInstance->BindContainer(CurrentInteractingContainer);

			// 마우스 활성화
			bShowMouseCursor = true;
			SetInputMode(FInputModeGameAndUI());

			UE_LOG(LogTemp, Warning, TEXT("공방 UI 열기"));
		}
	}
}
