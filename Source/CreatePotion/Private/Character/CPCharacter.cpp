// CPCharacter.cpp


#include "Character/CPCharacter.h"
#include "Character/CPInteractionComponent.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/HUD.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "CreatePotion.h"
#include "Character/CPPlayerController.h"
#include "GameCore/Interface/CPLevelUIInterface.h"

#include "UI/Widgets/Common/Container/CPContainerMainWidget.h"		// Container
#include "Components/CPInventoryComponent.h"						// Container
#include "GameInstance/Subsystem/CPInventorySubsystem.h"			// Container Subsystem


ACPCharacter::ACPCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// InteractionComponent
	InteractionComponent = CreateDefaultSubobject<UCPInteractionComponent>(TEXT("InteractionComponent"));

	// 인벤토리 컴포넌트
	InventoryComponent = CreateDefaultSubobject<UCPInventoryComponent>(TEXT("CPPlayerInventoryComponent"));
}

void ACPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACPCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ACPCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACPCharacter::Look);
		
		// Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ACPCharacter::OnInteractPressed);
		
		// Quest Toggling
		EnhancedInputComponent->BindAction(QuestToggleAction, ETriggerEvent::Started, this, &ACPCharacter::OnQuestTogglePressed);
	
		// 인벤토리 Toggle
		if (ToggleInventoryAction)
		{
			EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &ACPCharacter::ToggleInventoryUI);
		}
	}
	else
	{
		UE_LOG(LogCreatePotion, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACPCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (InventoryComponent)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UCPInventorySubsystem* InvSubsystem = GI->GetSubsystem<UCPInventorySubsystem>())
			{
				// 백업된 데이터가 있다면 InventoryComponent의 배열을 덮어씌우기
				InvSubsystem->LoadInventoryData(InventoryComponent->ContainerItems);
			}
		}
	}

	// 위젯 생성 및 초기 설정
	if (InventoryUIClass)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			// 위젯 생성
			InventoryUIInstance = CreateWidget<UCPContainerMainWidget>(PC, InventoryUIClass);
			if (InventoryUIInstance)
			{
				InventoryUIInstance->AddToViewport();

				// 인벤토리 컴포넌트와 UI 연동
				InventoryUIInstance->BindContainer(InventoryComponent);

				// 처음엔 Collapsed 처리
				InventoryUIInstance->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}

void ACPCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ACPCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ACPCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ACPCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ACPCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ACPCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ACPCharacter::OnInteractPressed()
{
	if (InteractionComponent)
	{
		InteractionComponent->TryInteract();
	}
}

void ACPCharacter::OnQuestTogglePressed()
{
	UE_LOG(LogTemp, Warning, TEXT("Tab키 입력"));
	
	ACPPlayerController* PC = Cast<ACPPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		PC->OnQuestTogglePressed();
	}
}

void ACPCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InventoryComponent)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UCPInventorySubsystem* InvSubsystem = GI->GetSubsystem<UCPInventorySubsystem>()) // 게임 인스턴스를 통해 서브시스템에 접근
			{
				InvSubsystem->SaveInventoryData(InventoryComponent->ContainerItems);
				// 현재 인벤토리 컴포넌트의 배열 데이터를 통째로 서브시스템에 저장
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

#pragma region Container
void ACPCharacter::ToggleInventoryUI()
{
	if (!InventoryUIInstance)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	// 현재 숨겨져 있는 상태라면 -> 열기 (Visible)
	if (InventoryUIInstance->GetVisibility() == ESlateVisibility::Collapsed ||
		InventoryUIInstance->GetVisibility() == ESlateVisibility::Hidden)
	{
		InventoryUIInstance->SetVisibility(ESlateVisibility::Visible);

		// 마우스 켜기 & Game And UI 모드 전환
		PC->bShowMouseCursor = true;

		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(InventoryUIInstance->TakeWidget()); // UI에 포커스 맞추기
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
	// 현재 열려 있는 상태라면 -> 닫기 (Collapsed)
	else
	{
		InventoryUIInstance->SetVisibility(ESlateVisibility::Collapsed);

		// 마우스 숨기기 & Game Only 모드 전환
		PC->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}
}

#pragma endregion
