#pragma once

#include "CoreMinimal.h"
#include "Data/CPForageableItemData.h"
#include "CPResourceType.generated.h"

// 채집 포인트 식별
USTRUCT(BlueprintType)
struct CREATEPOTION_API FCPResourceNodeKey
{
	GENERATED_BODY()
	
public:
	// 채집 포인트가 속한 레벨
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName LevelId = NAME_None;
	
	// 채집 포인트가 속한 스포너
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGuid SpawnerId;
	
	// 스포너 내에서 인덱스 번호
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 SlotIndex = INDEX_NONE;
	
	// 유효성 검사
	bool IsValid() const
	{
		return LevelId != NAME_None
			&& SpawnerId.IsValid()
			&& SlotIndex != INDEX_NONE;
	}
	
	// 채집 포인트가 일치하는가
	bool operator==(const FCPResourceNodeKey& Other) const
	{
		return LevelId == Other.LevelId
			&& SpawnerId == Other.SpawnerId
			&& SlotIndex == Other.SlotIndex;
	}
	
	// 해시 값 변환
	friend uint32 GetTypeHash(const FCPResourceNodeKey& Key)
	{
		uint32 Hash = GetTypeHash(Key.LevelId);
		Hash = HashCombine(Hash, GetTypeHash(Key.SpawnerId));
		Hash = HashCombine(Hash, GetTypeHash(Key.SlotIndex));
		
		return Hash;
	};
};

// 채집 포인트의 상태
USTRUCT(BlueprintType)
struct CREATEPOTION_API FCPResourceNodeState
{
	GENERATED_BODY()
	
public:
	// 다음 리스폰 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int64 RespawnAt = 0;
	
	// 레벨 전환 시 이전의 위치 정보를 저장하기 위한 값
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Generation = 0;
};

// 채집 대상 데이터
USTRUCT(BlueprintType)
struct CREATEPOTION_API FCPHarvestData
{
	GENERATED_BODY()
	
	// 채집 아이템 DA
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCPForageableItemData> HarvestedItem;
	
	// 채집량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 HarvestAmount = 1;
	
	// 채집 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float HarvestDuration = 1.f;
};