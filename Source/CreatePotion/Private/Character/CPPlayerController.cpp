// CPPlayerController.cpp

#include "Character/CPPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"

#include "EngineUtils.h"                          // TActorIterator
#include "NPC/Components/CPShopContainerComponent.h"

#include "CreatePotion.h"   // 로그용 헤더
#include "Character/CPCarryComponent.h"
#include "Components/CPItemContainerComponent.h"
#include "Components/CPInventoryComponent.h"
#include "Components/CPHandItemContainerComponent.h"
#include "Components/CPDropContainerComponent.h"

#include "UI/Context/CPContextInventoryOnly.h"
#include "UI/Widgets/Common/Container/CPContainerMainWidget.h"
#include "UI/Widgets/Common/Container/CPContainerGridWidget.h"
#include "UI/Widgets/Common/Container/CPHandHeldItemWidget.h"
#include "UI/Widgets/Common/Container/CPItemWheelWidget.h"
#include "UI/Widgets/Common/Container/Shop/CPShopPurchasePopupWidget.h"
#include "UI/Widgets/Common/Container/CPItemDetailPopupWidget.h"
#include "UI/Widgets/Common/Container/CPDropZoneWidget.h"

//디버그용 포션 Spawn 용 include
#include "Character/CPCarryComponent.h"
#include "GameInstance/Subsystem/CPSaveSubsystem.h"
#include "Lab/Actor/CPPotionActor.h"
#include "Settings/CPUISettings.h"
#include "UI/Widgets/Common/KeyGuide/CPThrowKeyGuideWidget.h"

void ACPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetContextHandlerForTargetContext(EUITargetContext::Inventory);
	
	InitKeyGuideWidget();
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

			if (IMC_PotionWheel)
			{
				Subsystem->AddMappingContext(IMC_PotionWheel, 10);
			}
		}
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_ScrollItem)
		{
			EIC->BindAction(IA_ScrollItem, ETriggerEvent::Triggered, this, &ACPPlayerController::OnItemWheelScroll);
		}

		if (IA_CarryHeldProp)
		{
			EIC->BindAction(IA_CarryHeldProp, ETriggerEvent::Started, this, &ACPPlayerController::OnCarryHeldProp);
		}
	}
}

#pragma region PotionThrow
void ACPPlayerController::ConsumeHeldPotionCharge()
{
	if (CachedInventoryComponent && bCurrentHeldItemIsFromInventory && CurrentlyHeldPotionInstance.IsValid())
	{
		// 저장해둔 인덱스 대신 같은 포션이 들어있는 칸을 찾아서 직접 차감
		const FContainerItem* Match = CachedInventoryComponent->ContainerItems.FindByPredicate(
			[this](const FContainerItem& It)
			{
				return UCPItemContainerComponent::AreInstancesStackable(It.Instance, CurrentlyHeldPotionInstance);
			});

		if (Match)
		{
			CachedInventoryComponent->RemoveItemFromContainer(Match->GridIndex, 1);
		}
	}

	CurrentlyHeldPotionInstance = FCPItemInstance();
	CurrentlyHeldPotionOriginIndex = -1;
	bCurrentHeldItemIsFromInventory = false;
	UpdatePotionHighlight();
}
#pragma endregion

#pragma region CarryProp
void ACPPlayerController::HandlePropAttached(ACPThrowablePropBase* Prop)
{
	
	// Q/E 장착 경로는 이미 인벤토리에 있으므로 무시
	if (bIsEquippingFromInventory || !IsValid(Prop) || !CachedInventoryComponent)
	{
		return;
	}

	const FCPItemInstance& Instance = Prop->GetWorkingIngredient();
	
	// 포션 이외의 아이템은 적용 x
	if (!Instance.IsValid() || Instance.CurrentEffects.Num() == 0)
	{
		return;
	}

	int32 Leftover = CachedInventoryComponent->TryGetItemFromInstance(Instance, 1);
	if (Leftover > 0)
	{
		// 인벤토리가 가득 차면 기존처럼 "필드에서 주운 것"으로 남겨둠 
		UE_LOG(LogContainer, Warning, TEXT("[Carry] 인벤토리 공간 부족으로 주운 포션을 인벤토리에 넣지 못함"));
		return;
	}

	// Q/E로 장착한 것과 동일하게 취급
	CurrentlyHeldPotionInstance = Instance;
	bCurrentHeldItemIsFromInventory = true;
	UpdatePotionHighlight();
}

void ACPPlayerController::UpdatePotionHighlight()
{
	if (!CachedInventoryComponent)
	{
		return;
	}

	UCPContainerMainWidget* InventoryUI = CachedInventoryComponent->GetInventoryUIInstance();
	if (!InventoryUI)
	{
		return;
	}

	int32 HighlightIndex = -1;

	bool bIsHoldingPotion = false;
	if (APawn* ControlledPawn = GetPawn())
	{
		if (UCPCarryComponent* CarryComponent = ControlledPawn->FindComponentByClass<UCPCarryComponent>())
		{
			bIsHoldingPotion = bCurrentHeldItemIsFromInventory && CarryComponent->HasHeldProp();
		}
	}

	// 손에 든 포션과 같은 포션이 들어있는 칸을 찾음
	if (bIsHoldingPotion && CurrentlyHeldPotionInstance.IsValid())
	{
		const FContainerItem* Match = CachedInventoryComponent->ContainerItems.FindByPredicate(
			[this](const FContainerItem& It)
			{
				return UCPItemContainerComponent::AreInstancesStackable(It.Instance, CurrentlyHeldPotionInstance);
			});

		if (Match)
		{
			HighlightIndex = Match->GridIndex;
		}
	}

	InventoryUI->SetHighlightedGridIndex(HighlightIndex);
}

void ACPPlayerController::HandleFocusedPotionChanged(int32 GridIndex)
{
	UpdatePotionHighlight();
}
#pragma endregion

#pragma region PotionWheel
void ACPPlayerController::OnItemWheelScroll(const FInputActionValue& Value)
{
	if (!ItemWheelWidgetInstance)
	{
		return;
	}
	bool bIsHoldingSomething = false;
	if (APawn* ControlledPawn = GetPawn())
	{
		if (UCPCarryComponent* CarryComponent = ControlledPawn->FindComponentByClass<UCPCarryComponent>())
		{
			bIsHoldingSomething = CarryComponent->HasHeldProp();
		}
	}

	// 아무것도 안 들고 있으면 휠을 넘기지 않고 현재 포커스 중인(=마지막으로 사용한) 포션을 장착
	if (!bIsHoldingSomething)
	{
		int32 FocusedGridIndex;
		if (ItemWheelWidgetInstance->GetFocusedPotionGridIndex(FocusedGridIndex))
		{
			ItemWheelWidgetInstance->ShowCurrentFocus();
			EquipItemAtGridIndex(FocusedGridIndex);
		}
		return;
	}

	int32 TargetGridIndex;
	if (!ItemWheelWidgetInstance->TryScrollAndPickNextFocus(Value.Get<float>(), TargetGridIndex))
	{
		return; // 슬라이드 진행 중이거나 포션이 없으면 아무 것도 안 함
	}

	EquipItemAtGridIndex(TargetGridIndex);
}

void ACPPlayerController::EquipItemAtGridIndex(int32 GridIndex)
{
	if (!CachedInventoryComponent)
	{
		return;
	}

	FContainerItem* FoundItem = CachedInventoryComponent->ContainerItems.FindByPredicate(
		[GridIndex](const FContainerItem& It) { return It.GridIndex == GridIndex; });

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
	if (!CarryComponent)
	{
		return;
	}

	// 그냥 손에 든 액터만 파괴
	if (CarryComponent->HasHeldProp())
	{
		if (ACPThrowablePropBase* PrevProp = CarryComponent->GetHeldProp())
		{
			CarryComponent->DetachProp(PrevProp, PrevProp->GetActorLocation()); // 참조만 해제
			PrevProp->Destroy(); // 인벤토리엔 원래 아이템이 그대로 있으니 손에 있던 액터만 파괴
		}
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

	bIsEquippingFromInventory = true;
	const bool bAttached = CarryComponent->AttachProp(Potion);
	bIsEquippingFromInventory = false;

	if (!bAttached)
	{
		Potion->Destroy();
		return;
	}

	// 인벤토리에서 차감은 하지 않고 "지금 이걸 들고 있다"는 정보만 저장
	CurrentlyHeldPotionInstance = FoundItem->Instance;
	CurrentlyHeldPotionOriginIndex = GridIndex;
	bCurrentHeldItemIsFromInventory = true;
	UpdatePotionHighlight();
}
#pragma endregion

// TODO[Container] : 추후 멀티플레이 확장시 Dedicated Server 환경에서 Client들은 실행이 되지 않음
void ACPPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (InPawn)
	{
		if (UGameInstance* GI = GetGameInstance()){
			if (UCPSaveSubsystem* SaveSubsystem = GI->GetSubsystem<UCPSaveSubsystem>()){
				SaveSubsystem->ApplyLoadPlayerTransform(InPawn);
			}
		}
		// 폰에 성공적으로 빙의되었을 때, 한 번만 찾아서 캐싱
		CachedInventoryComponent = InPawn->FindComponentByClass<UCPInventoryComponent>();
		LeftClickPickedContainer = InPawn->FindComponentByClass<UCPHandItemContainerComponent>();
		DropContainer = InPawn->FindComponentByClass<UCPDropContainerComponent>();

		// TODO[Container] : 레벨 전환시 자동 수거 기능을 구현하려면
		// 레벨에 배치된 별도의 Actor(또는 GameMode, GameState 등 레벨 단위로 바뀌는 객체)에 구현해야함
		if (ItemWheelWidgetClass && !ItemWheelWidgetInstance)
		{
			ItemWheelWidgetInstance = CreateWidget<UCPItemWheelWidget>(this, ItemWheelWidgetClass);
			if (ItemWheelWidgetInstance)
			{
				ItemWheelWidgetInstance->AddToViewport(-2);	// Visibility가 HitTestInvisible이라 영향 x
				ItemWheelWidgetInstance->BindInventory(CachedInventoryComponent);

				ItemWheelWidgetInstance->OnFocusedPotionChanged.AddUObject(this, &ACPPlayerController::HandleFocusedPotionChanged);
				ItemWheelWidgetInstance->BroadcastFocusedPotion();	// 최초 상태 Broadcast
			}
		}

		if (LeftClickPickedContainer && HandHeldItemWidgetClass && !HandHeldItemWidgetInstance)
		{
			HandHeldItemWidgetInstance = CreateWidget<UCPHandHeldItemWidget>(this, HandHeldItemWidgetClass);
			if (HandHeldItemWidgetInstance)
			{
				HandHeldItemWidgetInstance->AddToViewport(100); // 다른 UI보다 항상 위에 그려지도록
				HandHeldItemWidgetInstance->TryBindHandContainer(LeftClickPickedContainer);
			}
		}

		if (DropZoneWidgetClass && !DropZoneWidgetInstance)
		{
			DropZoneWidgetInstance = CreateWidget<UCPDropZoneWidget>(this, DropZoneWidgetClass);
			if (DropZoneWidgetInstance)
			{
				DropZoneWidgetInstance->OwnerContainer = DropContainer;
				DropZoneWidgetInstance->SlotGridIndex = 0;
				DropZoneWidgetInstance->AddToViewport(-1);	
			}
		}

		if (UCPCarryComponent* CarryComponent = InPawn->FindComponentByClass<UCPCarryComponent>())
		{
			CarryComponent->OnPropAttached.RemoveAll(this); // Delegate 중복 구독 방지를 위해 기존 항목 제거
			CarryComponent->OnPropAttached.AddUObject(this, &ACPPlayerController::HandlePropAttached);
			CarryComponent->OnHeldPropChanged.AddUniqueDynamic(this, &ACPPlayerController::HandleHeldPropChanged);
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

void ACPPlayerController::OnCarryHeldProp(const FInputActionValue& Value)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !CachedInventoryComponent)
	{
		return;
	}

	UCPCarryComponent* CarryComponent = ControlledPawn->FindComponentByClass<UCPCarryComponent>();
	if (!CarryComponent || !CarryComponent->HasHeldProp())
	{
		return;
	}

	if (bCurrentHeldItemIsFromInventory)
	{
		// 포션 : 인벤토리에 원본이 그대로 있으니 손만 비움 (차감 취소, 장착 취소)
		if (ACPThrowablePropBase* Prop = CarryComponent->GetHeldProp())
		{
			CarryComponent->DetachProp(Prop, Prop->GetActorLocation());
			Prop->Destroy();
		}
		CurrentlyHeldPotionInstance = FCPItemInstance();
		CurrentlyHeldPotionOriginIndex = -1;
	}
	else
	{
		// 월드에서 주운 재료 : 인벤토리에 새로 추가
		if (!CarryComponent->ConsumeHeldPropToInventory(CachedInventoryComponent))
		{
			UE_LOG(LogContainer, Warning, TEXT("[Carry] 인벤토리 공간 부족으로 수납 실패"));
		}
	}

	bCurrentHeldItemIsFromInventory = false;
	UpdatePotionHighlight();
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

	CurrentContainerUIInstance->AddToViewport(20);
	CurrentContainerUIInstance->BindContainer(InTargetContainer);
	CurrentInteractingContainer = InTargetContainer;

	// bShowMouseCursor = true;
	// SetInputMode(FInputModeGameAndUI());

	SetContextHandlerForTargetContext(InTargetContainer->TargetContext);

	if (UCPUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UCPUIManagerSubsystem>())
	{
		UIManager->RegisterCreatedWidget(CurrentContainerUIInstance);
		UIManager->PlayWidgetSound(CurrentContainerUIInstance->OpenSound);
	}
}

void ACPPlayerController::CloseExternalContainerUI()
{
	if (CurrentContainerUIInstance)
	{
		if (UCPUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UCPUIManagerSubsystem>())
		{
			UIManager->PlayWidgetSound(CurrentContainerUIInstance->CloseSound);
			UIManager->CloseWidget(CurrentContainerUIInstance);
		}
		
		CurrentContainerUIInstance = nullptr;
	}

	CurrentInteractingContainer = nullptr;

	// bShowMouseCursor = false;
	// SetInputMode(FInputModeGameOnly());

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

void ACPPlayerController::DebugToggleShopDirectly()
{
	UCPShopContainerComponent* FoundShop = nullptr;

	// TODO[Container] : 현재 Actor 순회를 통해 찾는 중, 추후 상호작용키로 변경 예정
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (UCPShopContainerComponent* Shop = It->FindComponentByClass<UCPShopContainerComponent>())
		{
			FoundShop = Shop;
			break; // 지금은 상점이 하나뿐인 상황을 전제로 작동, 여러 개면 비교 로직 필요
		}
	}

	if (!FoundShop)
	{
		UE_LOG(LogContainer, Warning, TEXT("[Debug] 월드에서 ShopContainerComponent를 찾지 못함"));
		return;
	}

	ToggleExternalContainerUI(FoundShop);
}
bool ACPPlayerController::BeginHoldingItem(const FContainerItem& InItem, UCPItemContainerComponent* OriginContainer, int32 OriginIndex)
{
	if (!LeftClickPickedContainer || IsHoldingItem() || !InItem.Instance.IsValid())
	{
		return false; // 이미 무언가 들고 있으면 새로 들 수 없음
	}

	FContainerItem HeldItem = InItem;
	HeldItem.GridIndex = 0; // HandHeld 컨테이너는 1칸만 존재하므로 항상 0번 index

	LeftClickPickedContainer->ContainerItems.Add(HeldItem);
	LeftClickPickedContainer->OnContainerUpdated.Broadcast();

	// 취소 시 되돌아갈 기존 Container의 종류와 index 정보 저장
	LeftClickPickedItemOriginContainer = OriginContainer;
	LeftClickPickedOriginSlotIndex = OriginIndex;

	// 집는 순간 열려있던 상세정보 UI가 있다면 닫기
	if (UCPItemDetailPopupWidget* Popup = JustGetItemDetailPopup())
	{
		Popup->HideDetail();
	}
	
	return true;
}

#pragma endregion

#pragma region Shop

UCPShopPurchasePopupWidget* ACPPlayerController::GetOrCreateShopPopup()
{
	if (!IsValid(ShopPopupInstance) && ShopPopupWidgetClass)
	{
		ShopPopupInstance = CreateWidget<UCPShopPurchasePopupWidget>(this, ShopPopupWidgetClass);
		if (ShopPopupInstance)
		{
			ShopPopupInstance->AddToViewport(200);
		}
	}
	return ShopPopupInstance;
}
#pragma endregion

#pragma region DetailWidget

UCPItemDetailPopupWidget* ACPPlayerController::GetOrCreateItemDetailPopup()
{
	if (!IsValid(ItemDetailPopupInstance) && ItemDetailPopupWidgetClass)
	{
		ItemDetailPopupInstance = CreateWidget<UCPItemDetailPopupWidget>(this, ItemDetailPopupWidgetClass);
		if (ItemDetailPopupInstance)
		{
			ItemDetailPopupInstance->AddToViewport(150);
		}
	}
	return ItemDetailPopupInstance;
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

void ACPPlayerController::InitKeyGuideWidget()
{
	// 키 가이드 위젯 생성 후 Visibility Collapsed로 설정
	const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
	if (!UISettings)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CPPlayerController][InitKeyGuideWidget]UISetting이 유효하지 않습니다."));
		return;
	}

	ThrowKeyGuideWidgetClass = UISettings->ThrowKeyGuideWidgetClass.LoadSynchronous();
	if (!IsValid(ThrowKeyGuideWidgetClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CPPlayerController][InitKeyGuideWidget]ThrowKeyGuideWidgetClass가 유효하지 않습니다."));
		return;
	}
	
	ThrowKeyGuideWidget = Cast<UCPThrowKeyGuideWidget>(CreateWidget(this, ThrowKeyGuideWidgetClass));
	UE_LOG(LogTemp, Warning, TEXT("[KeyGuide] Created = %s"), *GetNameSafe(ThrowKeyGuideWidget));
	
	if (!IsValid(ThrowKeyGuideWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CPPlayerController][InitKeyGuideWidget]ThrowKeyGuideWidget이 유효하지 않습니다."));
		return;
	}
	
	ThrowKeyGuideWidget->AddToViewport();
	ThrowKeyGuideWidget->HideWidget();
	
	
	
}

void ACPPlayerController::HandleHeldPropChanged(ACPThrowablePropBase* HeldProp)
{
	// 아이템 장착 중 키 가이드 UI 보여주기
	if (IsValid(HeldProp))
	{
		if (IsValid(ThrowKeyGuideWidget))
		{
			ThrowKeyGuideWidget->ShowWidget();			
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[CPPlayerController] ThrowKeyGuideWidget이 유효하지 않습니다."));
		}

	}
	else // 미장착 시 UI 끄기
	{
		if (IsValid(ThrowKeyGuideWidget))
		{
			ThrowKeyGuideWidget->HideWidget();	
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[CPPlayerController] ThrowKeyGuideWidget이 유효하지 않습니다."));
		}
	}
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
