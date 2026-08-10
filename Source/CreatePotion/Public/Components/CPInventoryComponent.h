// CPInventoryComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/CPItemContainerComponent.h"
#include "CPInventoryComponent.generated.h"

// UI 쪽에 '임시 인벤토리가 열려야 함'을 알리기 위한 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTempInventoryOpenedSignature);

UCLASS(ClassGroup = (CPContainer), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPInventoryComponent : public UCPItemContainerComponent
{
	GENERATED_BODY()
	
public:
	virtual int32 TryGetItem(UCPForageableItemData* InItemData, int32 Count) override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Inventory|Temp")
	TArray<FContainerItem> TempInventoryItems;

	// UI에서 블루프린트 이벤트로 바인딩할 수 있는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Container|Inventory|Event")
	FOnTempInventoryOpenedSignature OnTempInventoryOpened;
};
