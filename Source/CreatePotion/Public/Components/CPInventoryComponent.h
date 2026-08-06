// CPInventoryComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPInventoryComponent.generated.h"

// 인벤토리 Grid가 TryGetItem()을 통해 아이템을 얻을 수 없을 때
// 임시 인벤토리를 만드는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTempInventoryOpened);

class UCPForageableItemData;

USTRUCT(BlueprintType)
struct FInventoryItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    UCPForageableItemData* ItemDataAsset = nullptr;

    // 현재 개수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Stacked = 1;

    // 그리드 위치
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 GridIndex = -1;

    // 현재 아이템이 회전되어 있는지 여부, false = 정방향, true = 회전
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    bool bIsRotated = false;

    // Hash를 사용하여 Key값으로 정렬도 되지 않고 레플리케이션을 지원하지 않는 TMap 대신
    // 몇 가지 안되는 특성의 값을 직접 이용해서 정렬 구현
    // 이후에 속성이 늘어날 경우 배열로 관리
    // TODO : 인벤토리에서 Tag가 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Tag")
    int32 Tag_A = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Tag")
    int32 Tag_B = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Tag")
    int32 Tag_C = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Tag")
    int32 Tag_D = 0;
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Tag")
    //FString Tag_SP = 0;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCPInventoryComponent();

protected:
	virtual void BeginPlay() override;

private:
    // TODO : 인벤토리 내에서 같은 재료지만 Tag가 다른 재료를 저장할 일이 있을지?
    // 있다면 Tag를 추가 인자로 받아야할 가능성이 높음
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void TryGetItem(UCPForageableItemData* InItemData, int32 Count);

    bool FindAvailableSpace(int32 ItemWidth, int32 ItemHeight, int32& OutGridIndex);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FInventoryItem> InventoryItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Temp")
    TArray<FInventoryItem> TempInventoryItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Columns = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Rows = 5;

    FOnTempInventoryOpened OnTempInventoryOpened;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 MaxStack = 10;
};
