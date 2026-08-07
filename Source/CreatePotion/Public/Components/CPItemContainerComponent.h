// CPItemContainerComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/CPContainerTypes.h"
#include "CPItemContainerComponent.generated.h"

// 아이템 목록이 갱신되었음을 알리는 델리게이트, UI에 사용
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnContainerUpdatedSignature);

class UCPForageableItemData;

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

    // 특정 인덱스에 특정 크기(Width, Height)의 아이템을 넣을 수 있는지 2D 충돌 검사
    bool IsGridSpaceEnough(int32 TargetIndex, int32 ItemWidth, int32 ItemHeight) const;

    // 2D 그리드 검색 (아이템 고유 크기 사용)
    int32 FindGridSpace(UCPForageableItemData* ItemData, bool& bOutIsRotated);

protected:
	virtual void BeginPlay() override;

private:
    // 1D 슬롯 검색 (크기 무시, 무조건 1칸)
    bool FindSlotSpace(int32& OutGridIndex);

public:
    // 컨테이너 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings")
    EContainerType ContainerType = EContainerType::Grid2D;

    // Slot 모드일 때 사용할 총 칸 수 (가공 칸이 3칸이면 3)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings", meta = (EditCondition = "ContainerType == EContainerType::Slot1D"))
    int32 MaxSlots = 3;

    // Grid 모드일 때 사용할 가로/세로 크기
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings", meta = (EditCondition = "ContainerType == EContainerType::Grid2D"))
    int32 Columns = 3;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings", meta = (EditCondition = "ContainerType == EContainerType::Grid2D"))
    int32 Rows = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    TArray<FContainerItem> ContainerItems;

    // TODO : 재료 별 최대 스택 개수는 아이템 DataAsset에서 가져올 수 있도록
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container")
    int32 MaxStack = 10;

    UPROPERTY(BlueprintAssignable, Category = "Container|Event")
    FOnContainerUpdatedSignature OnContainerUpdated;
};
