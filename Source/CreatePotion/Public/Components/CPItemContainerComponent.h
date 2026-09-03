// CPItemContainerComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/CPContainerTypes.h"
#include "CPItemContainerComponent.generated.h"

// 아이템 목록이 갱신되었음을 알리는 델리게이트, UI에 사용
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnContainerUpdatedSignature);

class UCPForageableItemData;
class UCPContainerMainWidget;

UCLASS( ClassGroup=(CPContainer), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPItemContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCPItemContainerComponent();

    // DA만으로 비교 가능한 일반적인 재료
    UFUNCTION(BlueprintCallable, Category = "Container")
    virtual int32 TryGetItemFromData(UCPForageableItemData* InItemData, int32 Count);

    // Effects 등 추가 정보가 필요한 경우 (Potion 등)
    UFUNCTION(BlueprintCallable, Category = "Container")
    virtual int32 TryGetItemFromInstance(const FCPItemInstance& InInstance, int32 Count);

    // 특정 인덱스에 특정 크기(Width, Height)의 아이템을 넣을 수 있는지 2D 충돌 검사
    bool IsGridSpaceEnough(int32 TargetIndex, int32 ItemWidth, int32 ItemHeight) const;

    // 아이템 Swap이 가능한지를 체크하는 함수
    int32 FindItemArrayIndexCoveringGridIndex(int32 QueryIndex) const;

    // 2D 그리드 검색 (아이템 고유 크기 사용)
    int32 FindGridSpace(UCPForageableItemData* ItemData, bool& bOutIsRotated);

    UFUNCTION(BlueprintCallable, Category = "Container|Action")
    bool RemoveItemFromContainer(int32 TargetGridIndex, int32 AmountToRemove);

    // 컨테이너에서 아이템을 모두 꺼내는(집어드는) 함수
    UFUNCTION(BlueprintCallable, Category = "Container|Action")
    bool PopItemFromContainer(int32 TargetGridIndex, FContainerItem& OutPoppedItem);
    
    UFUNCTION(BlueprintCallable, Category = "Container|Action")
    bool AutoInsertItemToTargetContainer(int32 SourceGridIndex, UCPItemContainerComponent* TargetContainer);

    UFUNCTION(BlueprintCallable, Category = "Container|Action")
    bool TryPlaceHoldingItem(UCPItemContainerComponent* HandContainer, int32 TargetIndex);

    static bool AreInstancesStackable(const FCPItemInstance& A, const FCPItemInstance& B);
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

    // Slot 모드일 때 하나의 Slot이 사용할 Grid모드의 칸 단위의 width, height
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings", meta = (EditCondition = "ContainerType == EContainerType::Slot1D"))
    int32 SlotFitWidth = 2;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings", meta = (EditCondition = "ContainerType == EContainerType::Slot1D"))
    int32 SlotFitHeight = 2;

    // Grid 모드일 때 사용할 가로/세로 크기
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings", meta = (EditCondition = "ContainerType == EContainerType::Grid2D"))
    int32 Columns = 8;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings", meta = (EditCondition = "ContainerType == EContainerType::Grid2D"))
    int32 Rows = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    TArray<FContainerItem> ContainerItems;

    // TODO : 재료 별 최대 스택 개수는 아이템 DataAsset에서 가져올 수 있도록
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container")
    int32 MaxStack = 10;

    UPROPERTY(BlueprintAssignable, Category = "Container|Event")
    FOnContainerUpdatedSignature OnContainerUpdated;

#pragma region ExternalUI
    // 해당 컨테이너가 열렸을 때 어떤 Context을 사용할 지 결정할 때 쓰이는 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|UI")
    EUITargetContext TargetContext = EUITargetContext::Storage;

    // Context에 맞춰 생성될 UI 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|UI")
    TSubclassOf<UCPContainerMainWidget> ContainerUIClass;
#pragma endregion
};
