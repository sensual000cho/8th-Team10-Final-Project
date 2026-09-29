// CPPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Types/CPContainerTypes.h"
#include "CPPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;

class UCPItemContainerComponent;
class UCPInventoryComponent;
class UCPHandItemContainerComponent;
class UCPDropContainerComponent;

class UCPContainerContextBase;

class UCPContainerMainWidget;
class UCPHandHeldItemWidget;
class UCPItemWheelWidget;
class UCPWheelModeIndicatorWidget;
class UCPShopPurchasePopupWidget;
class UCPItemDetailPopupWidget;
class UCPDropZoneWidget;

//디버깅용 포션생성 함수용
class UCPForageableItemData;

UCLASS()
class CREATEPOTION_API ACPPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	friend class UCPContainerContextBase;	// private 멤버 변수에 접근할 수 있도록

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	// 폰 빙의가 변경될 때 엔진이 자동으로 호출해주는 함수
	// TODO[Container] : 추후 멀티플레이 확장시 Dedicated Server 환경에서 Client들은 실행이 되지 않음
	virtual void SetPawn(APawn* InPawn) override;

#pragma region DropItem
public:
	UPROPERTY(EditAnywhere, Category = "Container|UI")
	TSubclassOf<UCPDropZoneWidget> DropZoneWidgetClass;

	UPROPERTY()
	UCPDropContainerComponent* DropContainer;

private:
	UPROPERTY()
	UCPDropZoneWidget* DropZoneWidgetInstance;

#pragma endregion

#pragma region PotionThrow
public:
	void ConsumeHeldPotionCharge();
#pragma endregion

#pragma region CarryProp
private:
	void HandlePropAttached(ACPThrowablePropBase* Prop);

	void UpdatePotionHighlight();

private:
	// Q/E 장착으로 AttachProp을 호출하는 동안에만 true, 필드로부터 줍기와 구분용
	bool bIsEquippingFromInventory = false;
#pragma endregion

#pragma region Wheel
private:
	void OnItemWheelScroll(const FInputActionValue& Value);
	void EquipItemAtGridIndex(int32 GridIndex);

	void HandleFocusedPotionChanged(int32 GridIndex);

public:
	UPROPERTY(EditAnywhere, Category = "Input|Wheel")
	UInputAction* IA_ScrollItem; // Axis1D 휠 값

	UPROPERTY(EditAnywhere, Category = "Input|Wheel")
	UInputAction* IA_EquipFocusedItem; // 휠 클릭

protected:
	UPROPERTY(EditAnywhere, Category = "Input|Wheel")
	UInputMappingContext* IMC_PotionWheel;

private:
	UPROPERTY()
	FCPItemInstance CurrentlyHeldPotionInstance;

	int32 CurrentlyHeldPotionOriginIndex = -1;

	bool bCurrentHeldItemIsFromInventory = false;
#pragma endregion

#pragma region Container
public:
	// 대상 컨테이너의 ContainerUIClass를 읽어 UI를 열고 바인딩하는 범용 함수.
	// Lab, Storage, Shop 등 어떤 외부형 컨테이너든 동일하게 처리 가능.
	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	void OpenExternalContainerUI(UCPItemContainerComponent* InTargetContainer);

	// 현재 열려있는 외부 컨테이너 UI를 닫는 범용 함수
	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	void CloseExternalContainerUI();

	// 상호작용(F키 등)에서 바로 호출하기 좋은 토글 버전.
	// 이미 같은 컨테이너가 열려있으면 닫고, 아니면 엶.
	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	void ToggleExternalContainerUI(UCPItemContainerComponent* InTargetContainer);

	// 현재 아이템을 들고 있는지 확인하는 함수
	bool IsHoldingItem() const;

	// 아이템을 내려놓은 상태로 초기화 하는 함수 
	void ResetHoldingItem();

	UFUNCTION(BlueprintCallable, Category = "Debug|Shop")
	void DebugToggleShopDirectly();

	// 지정한 아이템을 HandItemContainer에 잡고 원래 위치 정보를 기록
	bool BeginHoldingItem(const FContainerItem& InItem, UCPItemContainerComponent* OriginContainer, int32 OriginIndex);

private:
	void SetContextHandlerForTargetContext(EUITargetContext InTargetContext);

	void OnCarryHeldProp(const FInputActionValue& Value);

protected:
	UPROPERTY(EditAnywhere, Category = "Input|Carry")
	UInputAction* IA_CarryHeldProp;

public:
	// 현재 상호작용 중인 컨테이너
	// TODO[Container] : F키 또는 컨테이너 이용 시 이 값을 해당 ItemContainerComponent로 설정해주어야 합니다.
	UPROPERTY(BlueprintReadWrite, Category = "Container")
	UCPItemContainerComponent* CurrentInteractingContainer = nullptr;

	// 마우스 클릭으로 "현재 들고 있는 아이템"을 보관할 Container
	UPROPERTY(BlueprintReadWrite, Category = "Container|Drag")
	UCPItemContainerComponent* LeftClickPickedContainer = nullptr;

	// Context를 처리할 Handler
	UPROPERTY()
	UCPContainerContextBase* CurrentContextHandler;

	// 캐싱할 Inventory Component
	UPROPERTY()
	UCPInventoryComponent* CachedInventoryComponent = nullptr;

	UPROPERTY(EditAnywhere, Category = "Container|UI")
	TSubclassOf<UCPHandHeldItemWidget> HandHeldItemWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Container|UI")
	TSubclassOf<UCPItemWheelWidget> ItemWheelWidgetClass;
protected:
	// 현재 화면에 떠 있는 외부 컨테이너 UI 인스턴스 (Lab, Storage, Shop 공용)
	UPROPERTY()
	UCPContainerMainWidget* CurrentContainerUIInstance;

private:
	UPROPERTY()
	UCPItemWheelWidget* ItemWheelWidgetInstance;

	UPROPERTY()
	UCPHandHeldItemWidget* HandHeldItemWidgetInstance;

	// 마우스 클릭으로 "현재 들고 있는 아이템"을 우클릭 등으로 취소할 경우 되돌아갈 기존 Container
	UPROPERTY()
	UCPItemContainerComponent* LeftClickPickedItemOriginContainer = nullptr;

	// 마우스 클릭으로 "현재 들고 있는 아이템"의 기존 Index
	int32 LeftClickPickedOriginSlotIndex = -1;
#pragma endregion

#pragma region Shop
public:
	UCPShopPurchasePopupWidget* GetOrCreateShopPopup();

	UCPShopPurchasePopupWidget* JustGetShopPopup() const { return ShopPopupInstance; }

public:
	UPROPERTY(EditAnywhere, Category = "Shop|UI")
	TSubclassOf<UCPShopPurchasePopupWidget> ShopPopupWidgetClass;

private:
	UPROPERTY()
	UCPShopPurchasePopupWidget* ShopPopupInstance;
#pragma endregion

#pragma region DetailWidget
public:
	UCPItemDetailPopupWidget* GetOrCreateItemDetailPopup();

	UCPItemDetailPopupWidget* JustGetItemDetailPopup() const { return ItemDetailPopupInstance; }

public:
	UPROPERTY(EditAnywhere, Category = "Container|UI")
	TSubclassOf<UCPItemDetailPopupWidget> ItemDetailPopupWidgetClass;

private:
	UPROPERTY()
	UCPItemDetailPopupWidget* ItemDetailPopupInstance;
#pragma endregion
	
//디버깅용 포션생성
#pragma region DebugPotionHotkeys
public:
	// BP에서 지정한 효과의 포션을 손에 생성한다.
	UFUNCTION(BlueprintCallable, Category = "Debug|Potion Hotkeys")
	void SpawnDebugPotion(UPARAM(meta = (Categories = "Potion.Effect")) const TArray<FGameplayTag>& EffectTags);

	UFUNCTION(BlueprintCallable, Category = "Debug|Potion Hotkeys")
	void DebugSpawnFartLaunchPotion();

	UFUNCTION(BlueprintCallable, Category = "Debug|Potion Hotkeys")
	void DebugSpawnGiantPotion();

	UFUNCTION(BlueprintCallable, Category = "Debug|Potion Hotkeys")
	void DebugSpawnCombinedPotion();

	UFUNCTION(BlueprintCallable, Category = "Debug|Potion Hotkeys")
	void DebugAddFartLaunchPotionToInventory();

	UFUNCTION(BlueprintCallable, Category = "Debug|Potion Hotkeys")
	void DebugAddGiantPotionToInventory();

	UFUNCTION(BlueprintCallable, Category = "Debug|Potion Hotkeys")
	void DebugAddCombinedPotionToInventory();
private:
	// TEMP: Cauldron과 레벨 이동을 우회하는 포션 생성 기능.
	UPROPERTY(EditDefaultsOnly, Category = "Debug|Potion Hotkeys")
	TObjectPtr<UCPForageableItemData> DebugPotionData;

#pragma endregion
};