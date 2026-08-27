// CPInventoryComponent.cpp

#include "Components/CPInventoryComponent.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "CreatePotion.h"   // 로그용 헤더

#include "GameInstance/Subsystem/CPContainerSubsystem.h"
#include "UI/Widgets/Common/Container/CPContainerMainWidget.h"

UCPInventoryComponent::UCPInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UCPContainerSubsystem* ContainerSubsystem = GI->GetSubsystem<UCPContainerSubsystem>())
		{
			ContainerSubsystem->LoadContainerData(this->ContainerItems);
		}
	}

	// UI 생성 및 연동
	if (InventoryUIClass)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			InventoryUIInstance = CreateWidget<UCPContainerMainWidget>(PC, InventoryUIClass);
			if (InventoryUIInstance)
			{
				InventoryUIInstance->AddToViewport();
				InventoryUIInstance->BindContainer(this); // 데이터 씌운 후 자신을 바인딩
				InventoryUIInstance->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}

	BindRetryCount = 0;
	GetWorld()->GetTimerManager().SetTimer(
		InputBindingTimerHandle, 
		this, 
		&UCPInventoryComponent::TryBindInput, 
		0.1f, 
		true
	);
}

void UCPInventoryComponent::TryBindInput()
{
	if (!ToggleInventoryAction)
	{
		// 액션이 할당 안 되어 있으면 시도할 필요조차 없으니 타이머 즉시 종료
		GetWorld()->GetTimerManager().ClearTimer(InputBindingTimerHandle);
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->InputComponent)
	{
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PC->InputComponent))
		{
			EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &UCPInventoryComponent::ToggleInventoryUI);

			// 바인딩 성공시 타이머 즉시 종료
			GetWorld()->GetTimerManager().ClearTimer(InputBindingTimerHandle);
			UE_LOG(LogContainer, Log, TEXT("[Inventory] 입력 바인딩 성공"));
		}
	}
	else
	{
		// 실패 시 시도한 횟수 카운트 증가
		BindRetryCount++;

		// 50회(약 5초)가 지나도 안 되면 몬스터나 NPC에 달린 컴포넌트로 간주하고 포기
		if (BindRetryCount >= 50)
		{
			GetWorld()->GetTimerManager().ClearTimer(InputBindingTimerHandle);
			UE_LOG(LogTemp, Warning, TEXT("[Inventory] 입력 바인딩 포기"));
		}
	}
}

void UCPInventoryComponent::ToggleInventoryUI()
{
	UE_LOG(LogTemp, Warning, TEXT("[Inventory] Toggle"));

	if (!InventoryUIInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[Inventory] UI Instance를 찾을 수 없음"));
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	if (InventoryUIInstance->GetVisibility() == ESlateVisibility::Collapsed ||
		InventoryUIInstance->GetVisibility() == ESlateVisibility::Hidden)
	{
		InventoryUIInstance->SetVisibility(ESlateVisibility::Visible);

		PC->bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(InventoryUIInstance->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
	else
	{
		InventoryUIInstance->SetVisibility(ESlateVisibility::Collapsed);

		PC->bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}
}

int32 UCPInventoryComponent::TryGetItem(UCPForageableItemData* InItemData, int32 Count)
{
	int32 LeftoverCount = Super::TryGetItem(InItemData, Count);

	// 가방이 꽉 차서 남은 아이템이 반환되었을 때 임시 인벤토리에 저장
	if (LeftoverCount > 0)
	{
		FContainerItem TempItem;
		TempItem.ItemDataAsset = InItemData;
		TempItem.Stacked = LeftoverCount;
		TempItem.GridIndex = -1; // 임시 인벤토리임을 나타내는 인덱스

		TempInventoryItems.Add(TempItem);

		UE_LOG(LogContainer, Warning, TEXT("남은 %d개가 임시 인벤토리로 이동되었습니다."), LeftoverCount);

		// UI 쪽에 임시 인벤토리 팝업창을 띄우라고 Broadcast
		OnTempInventoryOpened.Broadcast();

		// TODO : 현재는 무조건 임시 인벤토리로 모두 넘기고 있습니다.
		// 일단은 임시 인벤토리가 남은 수량을 모두 흡수했으므로, 
		// "더 이상 필드에 떨어뜨릴 아이템은 없다"는 의미로 0을 반환
		return 0;
	}

	// 남은 게 없었다면 그대로 0 반환
	return 0;
}

void UCPInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UCPContainerSubsystem* ContainerSubsystem = GI->GetSubsystem<UCPContainerSubsystem>())
		{
			ContainerSubsystem->SaveContainerData(this->ContainerItems);
		}
	}

	GetWorld()->GetTimerManager().ClearTimer(InputBindingTimerHandle);

	Super::EndPlay(EndPlayReason);
}

bool UCPInventoryComponent::TryGetMoney(int32 InAmount)
{
	if (InAmount <= 0)
	{
		UE_LOG(LogContainer, Warning, TEXT("[Money] TryGetMoney 실패, 0 이하의 금액을 획득 시도 : %d"), InAmount);
		return false;
	}

	if (OwningMoney >= MaxMoney)
	{
		UE_LOG(LogContainer, Warning, TEXT("[Money] TryGetMoney 실패: 최대 소지 금액 한도에 도달 (%d / %d)"), OwningMoney, MaxMoney);
		return false;
	}

	// 초과분은 잘라내고, 최대치(MaxMoney)까지만 채움
	int32 OldAmount = OwningMoney;
	OwningMoney = FMath::Min(OwningMoney + InAmount, MaxMoney);

	UE_LOG(LogContainer, Log, TEXT("[Money] 소지금 증가: %d -> %d (요청: +%d)"), OldAmount, OwningMoney, InAmount);
	OnMoneyChanged.Broadcast(OwningMoney);
	return true;
}

bool UCPInventoryComponent::TrySpendMoney(int32 InAmount)
{
	if (InAmount <= 0)
	{
		UE_LOG(LogContainer, Warning, TEXT("[Money] TrySpendMoney 실패: 0 이하의 금액을 소비 시도 : %d"), InAmount);
		return false;
	}

	if (OwningMoney < InAmount)
	{
		UE_LOG(LogContainer, Warning, TEXT("[Money] TrySpendMoney 실패: 소지금 부족 (보유 %d / 필요 %d)"), OwningMoney, InAmount);
		return false;
	}

	int32 OldAmount = OwningMoney;
	OwningMoney -= InAmount;

	UE_LOG(LogContainer, Log, TEXT("[Money] 소지금 지출: %d -> %d (요청: -%d)"), OldAmount, OwningMoney, InAmount);
	OnMoneyChanged.Broadcast(OwningMoney);
	return true;
}
