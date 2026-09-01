// CPInventoryComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/CPItemContainerComponent.h"
#include "CPInventoryComponent.generated.h"

class UInputMappingContext;
class UInputAction;

class UCPContainerMainWidget;

// '임시 인벤토리가 열려야 함'을 알리는 UI 갱신 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTempInventoryOpenedSignature);

// 소지금이 변경되었음을 알리는 UI 갱신 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoneyChangedSignature, int32, NewAmount);

UCLASS(ClassGroup = (CPContainer), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPInventoryComponent : public UCPItemContainerComponent
{
	GENERATED_BODY()
	
public:
	UCPInventoryComponent();

	virtual int32 TryGetItemFromData(UCPForageableItemData* InItemData, int32 Count) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma region Money
public:
	// 소지금을 늘림 (아이템 판매, 보상 획득 등)
	// 최대 소지 금액을 초과하는 경우 실패
	UFUNCTION(BlueprintCallable, Category = "Inventory|Money")
	bool TryGetMoney(int32 InAmount);

	// 소지금을 지출함 (아이템 구매 등)
	// 소지금이 부족한 경우 실패
	UFUNCTION(BlueprintCallable, Category = "Inventory|Money")
	bool TrySpendMoney(int32 InAmount);

	void RestoreMoney(int32 InAmount);

	UFUNCTION(BlueprintPure, Category = "Inventory|Money")
	int32 GetOwningMoney() const { return OwningMoney; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Money")
	int32 GetMaxMoney() const { return MaxMoney; }

public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Money|Event")
	FOnMoneyChangedSignature OnMoneyChanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Money")
	int32 OwningMoney = 0;
protected:
	// 소지 가능한 최대 금액
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Money", meta = (ClampMin = "0"))
	int32 MaxMoney = 50000;
#pragma endregion
	
#pragma region UI
public:
	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	void ToggleInventoryUI();

	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	void OpenInventoryUI();

	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	void CloseInventoryUI();

private:
	void TryBindInput();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|UI")
	TSubclassOf<UCPContainerMainWidget> InventoryUIClass;

	UPROPERTY(EditAnywhere, Category = "Container|Input")
	UInputAction* ToggleInventoryAction;

private:
	UPROPERTY()
	UCPContainerMainWidget* InventoryUIInstance;

	// TryBindInput()을 retry하기 위한 타이머 핸들
	FTimerHandle InputBindingTimerHandle;

	// 무한 루프를 방지하기 위한 최대 횟수 카운트용
	int32 BindRetryCount = 0;
#pragma endregion

#pragma region TempInventory
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Inventory|Temp")
	TArray<FContainerItem> TempInventoryItems;

	// UI에서 블루프린트 이벤트로 바인딩할 수 있는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Container|Inventory|Event")
	FOnTempInventoryOpenedSignature OnTempInventoryOpened;
#pragma endregion
};
