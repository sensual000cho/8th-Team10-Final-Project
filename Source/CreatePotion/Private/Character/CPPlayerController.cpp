// CPPlayerController.cpp

#include "Character/CPPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "GameCore/Interface/CPLevelUIInterface.h"
#include "GameFramework/HUD.h"

#include "CreatePotion.h"   // 로그용 헤더
#include "UI/Widgets/Common/Container/CPContainerMainWidget.h"
#include "UI/Widgets/Common/Container/CPContainerGridWidget.h"
#include "UI/Widgets/Common/Container/CPHandHeldItemWidget.h"
#include "UI/Widgets/Common/Container/CPItemWheelWidget.h"
#include "Components/CPItemContainerComponent.h"
#include "Components/CPInventoryComponent.h"
#include "Components/CPHandItemContainerComponent.h"
#include "UI/Context/CPContextInventoryOnly.h"

//디버그용 포션 Spawn 용 include
#include "Character/CPCarryComponent.h"
#include "Lab/Actor/CPPotionActor.h"


void ACPPlayerController::SetWheelInputMode(EWheelInputMode NewMode)
{
	if (CurrentWheelMode == NewMode)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem)
	{
		return;
	}

	// 이전에 켜져 있던 전용 IMC만 끔 (캐릭터 BP가 등록한 기본 줌 IMC는 절대 건드리지 않음)
	if (CurrentWheelMode == EWheelInputMode::PotionSelect && IMC_PotionWheel)
	{
		Subsystem->RemoveMappingContext(IMC_PotionWheel);
	}
	else if (CurrentWheelMode == EWheelInputMode::IngredientSelect && IMC_IngredientWheel)
	{
		Subsystem->RemoveMappingContext(IMC_IngredientWheel);
	}

	CurrentWheelMode = NewMode;

	// 새 모드의 전용 IMC를 더 높은 우선순위로 등록
	if (NewMode == EWheelInputMode::PotionSelect && IMC_PotionWheel)
	{
		Subsystem->AddMappingContext(IMC_PotionWheel, 10); // 캐릭터 IMC보다 높게
	}
	else if (NewMode == EWheelInputMode::IngredientSelect && IMC_IngredientWheel)
	{
		Subsystem->AddMappingContext(IMC_IngredientWheel, 10);
	}
}

void ACPPlayerController::CycleWheelInputMode()
{
	uint8 NextModeValue = (static_cast<uint8>(CurrentWheelMode) + 1) % 3;
	SetWheelInputMode(static_cast<EWheelInputMode>(NextModeValue));

	FString ModeName;
	switch (CurrentWheelMode)
	{
	case EWheelInputMode::CameraZoom:        ModeName = TEXT("카메라 줌"); break;
	case EWheelInputMode::PotionSelect:      ModeName = TEXT("포션 선택"); break;
	case EWheelInputMode::IngredientSelect:  ModeName = TEXT("재료 선택"); break;
	}

	// TODO[Container] : 현재 Mode UI 표시
}

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

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_ScrollItem)
		{
			EIC->BindAction(IA_ScrollItem, ETriggerEvent::Triggered, this, &ACPPlayerController::OnItemWheelScroll);
		}

		if (IA_EquipFocusedItem)
		{
			EIC->BindAction(IA_EquipFocusedItem, ETriggerEvent::Started, this, &ACPPlayerController::OnEquipFocusedItem);
		}

		if (IA_CycleWheelMode)
		{
			EIC->BindAction(IA_CycleWheelMode, ETriggerEvent::Started, this, &ACPPlayerController::CycleWheelInputMode);
		}
	}
}

void ACPPlayerController::OnItemWheelScroll(const FInputActionValue& Value)
{
	if (ItemWheelWidgetInstance)
	{
		ItemWheelWidgetInstance->HandleScrollInput(Value.Get<float>());
	}
}

void ACPPlayerController::OnEquipFocusedItem(const FInputActionValue& Value)
{
	if (!ItemWheelWidgetInstance || !CachedInventoryComponent)
	{
		return;
	}

	int32 FocusedGridIndex;
	if (!ItemWheelWidgetInstance->GetFocusedPotionGridIndex(FocusedGridIndex))
	{
		return; // 휠에 아무것도 없으면 할 일 없음
	}

	// GridIndex로 실제 아이템 데이터 찾기
	FContainerItem* FoundItem = CachedInventoryComponent->ContainerItems.FindByPredicate(
		[FocusedGridIndex](const FContainerItem& It) { return It.GridIndex == FocusedGridIndex; });

	if (!FoundItem || !FoundItem->Instance.SourceItemData)
	{
		return;
	}

	UWorld* World = GetWorld();
	APawn* ControlledPawn = GetPawn();
	if (!World || !ControlledPawn)
	{
		return;
	}

	UCPCarryComponent* CarryComponent = ControlledPawn->FindComponentByClass<UCPCarryComponent>();
	if (!CarryComponent || CarryComponent->HasHeldProp())
	{
		return; // 이미 손에 뭔가 들고 있으면 장착 불가
	}

	UCPForageableItemData* PotionData = FoundItem->Instance.SourceItemData;
	UClass* PotionActorClass = PotionData->AlchemyPropClass.LoadSynchronous();
	if (!PotionActorClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACPPotionActor* Potion = World->SpawnActor<ACPPotionActor>(
		PotionActorClass, CarryComponent->GetComponentTransform(), SpawnParameters);
	if (!Potion)
	{
		return;
	}

	Potion->InitializeFromItemData(PotionData, FoundItem->Instance.CurrentEffects);

	if (!CarryComponent->AttachProp(Potion))
	{
		Potion->Destroy();
		return;
	}

	// 장착 성공했으니 인벤토리에서 1개 차감
	CachedInventoryComponent->RemoveItemFromContainer(FocusedGridIndex, 1);
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

		// TODO[Container] : 레벨 전환시 자동 수거 기능을 구현하려면
		// 레벨에 배치된 별도의 Actor(또는 GameMode, GameState 등 레벨 단위로 바뀌는 객체)에 구현해야함

		if (CachedInventoryComponent)
		{
			UE_LOG(LogContainer, Log, TEXT("인벤토리 컴포넌트 캐싱 완료"));
		}

		if (ItemWheelWidgetClass && !ItemWheelWidgetInstance)
		{
			ItemWheelWidgetInstance = CreateWidget<UCPItemWheelWidget>(this, ItemWheelWidgetClass);
			if (ItemWheelWidgetInstance)
			{
				ItemWheelWidgetInstance->AddToViewport();
				UE_LOG(LogContainer, Warning, TEXT("[ItemWheel] AddToViewport 호출됨, IsInViewport=%d"), ItemWheelWidgetInstance->IsInViewport());
				ItemWheelWidgetInstance->BindInventory(CachedInventoryComponent);
			}

			UE_LOG(LogContainer, Log, TEXT("아이템 휠 스크롤 UI 바인딩 완료"));
		}
		else
		{
			UE_LOG(LogContainer, Error, TEXT("[ItemWheel] 위젯 생성 실패: Inventory=%d, Class=%d, AlreadyExists=%d"),
				CachedInventoryComponent != nullptr, ItemWheelWidgetClass != nullptr, ItemWheelWidgetInstance != nullptr);
		}

		if (LeftClickPickedContainer && HandHeldItemWidgetClass && !HandHeldItemWidgetInstance)
		{
			HandHeldItemWidgetInstance = CreateWidget<UCPHandHeldItemWidget>(this, HandHeldItemWidgetClass);
			if (HandHeldItemWidgetInstance)
			{
				HandHeldItemWidgetInstance->AddToViewport(100); // 다른 UI보다 항상 위에 그려지도록
				HandHeldItemWidgetInstance->TryBindHandContainer(LeftClickPickedContainer);
			}

			UE_LOG(LogContainer, Log, TEXT("아이템 집기 컴포넌트 캐싱 및 UI 바인딩 완료"));
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


#pragma region DebugPotionHotkeys
void ACPPlayerController::DebugSpawnFartLaunchPotion()
{
	TArray<FGameplayTag> EffectTags;
	EffectTags.Add(FGameplayTag::RequestGameplayTag(FName(TEXT("Potion.Effect.FartLaunch"))));

	SpawnDebugPotion(EffectTags);
}

void ACPPlayerController::DebugSpawnGiantPotion()
{
	TArray<FGameplayTag> EffectTags;
	EffectTags.Add(FGameplayTag::RequestGameplayTag(FName(TEXT("Potion.Effect.Giant"))));

	SpawnDebugPotion(EffectTags);
}

void ACPPlayerController::DebugSpawnCombinedPotion()
{
	TArray<FGameplayTag> EffectTags;
	EffectTags.Add(FGameplayTag::RequestGameplayTag(FName(TEXT("Potion.Effect.FartLaunch"))));
	EffectTags.Add(FGameplayTag::RequestGameplayTag(FName(TEXT("Potion.Effect.Giant"))));

	SpawnDebugPotion(EffectTags);
}

void ACPPlayerController::DebugAddFartLaunchPotionToInventory()
{
	if (!DebugPotionData || !CachedInventoryComponent)
	{
		UE_LOG(LogContainer, Warning, TEXT("[Debug] DebugPotionData 또는 인벤토리가 없습니다."));
		return;
	}

	FCPItemInstance Instance;
	Instance.SourceItemData = DebugPotionData;
	Instance.CurrentEffects.Add(FGameplayTag::RequestGameplayTag(FName(TEXT("Potion.Effect.FartLaunch"))));

	int32 Leftover = CachedInventoryComponent->TryGetItemFromInstance(Instance, 1);
	UE_LOG(LogContainer, Log, TEXT("[Debug] 포션 인벤토리 획득 시도 결과, 남은 개수: %d"), Leftover);
}

void ACPPlayerController::DebugAddGiantPotionToInventory()
{
	if (!DebugPotionData || !CachedInventoryComponent)
	{
		UE_LOG(LogContainer, Warning, TEXT("[Debug] DebugPotionData 또는 인벤토리가 없습니다."));
		return;
	}

	FCPItemInstance Instance;
	Instance.SourceItemData = DebugPotionData;
	Instance.CurrentEffects.Add(FGameplayTag::RequestGameplayTag(FName(TEXT("Potion.Effect.Giant"))));

	int32 Leftover = CachedInventoryComponent->TryGetItemFromInstance(Instance, 1);
	UE_LOG(LogContainer, Log, TEXT("[Debug] 포션 인벤토리 획득 시도 결과, 남은 개수: %d"), Leftover);
}

void ACPPlayerController::DebugAddCombinedPotionToInventory()
{
	if (!DebugPotionData || !CachedInventoryComponent)
	{
		UE_LOG(LogContainer, Warning, TEXT("[Debug] DebugPotionData 또는 인벤토리가 없습니다."));
		return;
	}

	FCPItemInstance Instance;
	Instance.SourceItemData = DebugPotionData;
	Instance.CurrentEffects.Add(FGameplayTag::RequestGameplayTag(FName(TEXT("Potion.Effect.FartLaunch"))));
	Instance.CurrentEffects.Add(FGameplayTag::RequestGameplayTag(FName(TEXT("Potion.Effect.Giant"))));

	int32 Leftover = CachedInventoryComponent->TryGetItemFromInstance(Instance, 1);
	UE_LOG(LogContainer, Log, TEXT("[Debug] 포션 인벤토리 획득 시도 결과, 남은 개수: %d"), Leftover);
}

void ACPPlayerController::SpawnDebugPotion(const TArray<FGameplayTag>& EffectTags)
{
	UWorld* World = GetWorld();
	APawn* ControlledPawn = GetPawn();

	if (!World || !ControlledPawn || !DebugPotionData)
	{
		return;
	}

	UCPCarryComponent* CarryComponent = ControlledPawn->FindComponentByClass<UCPCarryComponent>();

	if (!CarryComponent || CarryComponent->HasHeldProp())
	{
		return;
	}

	UClass* PotionActorClass = DebugPotionData->AlchemyPropClass.LoadSynchronous();

	if (!PotionActorClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACPPotionActor* Potion = World->SpawnActor<ACPPotionActor>(PotionActorClass, CarryComponent->GetComponentTransform(), SpawnParameters);
	if (!Potion)
	{
		return;
	}
	Potion->InitializeFromItemData(DebugPotionData, EffectTags);
	if (!CarryComponent->AttachProp(Potion))
	{
		Potion->Destroy();
	}
}


#pragma endregion