// CPInventoryComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/CPItemContainerComponent.h"
#include "CPInventoryComponent.generated.h"

class UInputMappingContext;
class UInputAction;

class UCPContainerMainWidget;

// UI 쪽에 '임시 인벤토리가 열려야 함'을 알리기 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTempInventoryOpenedSignature);

UCLASS(ClassGroup = (CPContainer), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPInventoryComponent : public UCPItemContainerComponent
{
	GENERATED_BODY()
	
public:
	UCPInventoryComponent();

	virtual int32 TryGetItem(UCPForageableItemData* InItemData, int32 Count) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
#pragma region UI
public:
	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	void ToggleInventoryUI();

private:
	void TryBindInput();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|UI")
	TSubclassOf<UCPContainerMainWidget> InventoryUIClass;

	UPROPERTY(EditAnywhere, Category = "Container|Input")
	UInputMappingContext* InventoryMappingContext;

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
