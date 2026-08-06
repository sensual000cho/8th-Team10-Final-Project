// CPItemContainerComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPItemContainerComponent.generated.h"

class UCPForageableItemData;

UENUM(BlueprintType)
enum class EContainerType : uint8
{
    Slot1D UMETA(DisplayName = "슬롯형 (가공, 장비창)"),
    Grid2D UMETA(DisplayName = "격자형 (인벤토리, 창고, 상점)")
};

USTRUCT(BlueprintType)
struct FContainerItem
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
    // 몇 가지 안되는 특성의 값을 캐싱해서 정렬 구현, 이후에 속성이 늘어날 경우 배열로 관리
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

UCLASS( ClassGroup=(CPContainer), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPItemContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCPItemContainerComponent();

    // TODO : 인벤토리 내에서 같은 재료지만 Tag가 다른 재료를 저장할 일이 있을지?
    // 있다면 Tag를 추가 인자로 받아야할 가능성이 높음
    UFUNCTION(BlueprintCallable, Category = "Container")
    virtual int32 TryGetItem(UCPForageableItemData* InItemData, int32 Count);

protected:
	virtual void BeginPlay() override;

private:
    // 1D 슬롯 검색 (크기 무시, 무조건 1칸)
    bool FindSlotSpace(int32& OutGridIndex);

    // 2D 그리드 검색 (아이템 고유 크기 사용)
    bool FindGridSpace(int32 ItemWidth, int32 ItemHeight, int32& OutGridIndex);

public:
    // 컨테이너 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings")
    EContainerType ContainerType = EContainerType::Grid2D;

    // Slot 모드일 때 사용할 총 칸 수 (가공 칸이 3칸이면 3)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings", meta = (EditCondition = "ContainerType == EContainerType::Slot1D"))
    int32 MaxSlots = 3;

    // Grid 모드일 때 사용할 가로/세로 크기
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings", meta = (EditCondition = "ContainerType == EContainerType::Grid2D"))
    int32 Columns = 8;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings", meta = (EditCondition = "ContainerType == EContainerType::Grid2D"))
    int32 Rows = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    TArray<FContainerItem> ContainerItems;

    // TODO : 재료 별 최대 스택 개수는 아이템 DataAsset에서 가져올 수 있도록
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container")
    int32 MaxStack = 10;
};
