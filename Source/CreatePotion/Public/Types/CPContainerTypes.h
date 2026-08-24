// CPContainerTypes.h

#pragma once

#include "CoreMinimal.h"
#include "Data/CPForageableItemData.h"
#include "CPContainerTypes.generated.h"

UENUM(BlueprintType)
enum class EContainerType : uint8
{
    Slot1D UMETA(DisplayName = "슬롯형 (가공, 장비창)"),
    Grid2D UMETA(DisplayName = "격자형 (인벤토리, 창고, 상점)")
};

UENUM(BlueprintType)
enum class EUITargetContext : uint8
{
    Hand            UMETA(DisplayName = "아이템 잡기"),
    Inventory       UMETA(DisplayName = "인벤토리 단독"),
    Lab             UMETA(DisplayName = "공방 가공기"),
    Storage         UMETA(DisplayName = "창고"),
    Shop            UMETA(DisplayName = "상점")
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
