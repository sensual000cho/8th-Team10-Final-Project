#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Resource/CPResourceType.h"
#include "CPResourceSpawner.generated.h"

class ACPResourceNodeActor;
class UCPResourceDefinition;
class UBoxComponent;

// 채집물 스폰 가중치를 위한 구조체
USTRUCT(BlueprintType)
struct FCPResourceSpawnEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCPResourceDefinition> ResourceDefinition;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	float Weight = 1.f;
};

UCLASS()
class CREATEPOTION_API ACPResourceSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ACPResourceSpawner();

protected:
	virtual void BeginPlay() override;
	
	// 초기 배치 시, 고유 Id 생성
	virtual void PostActorCreated() override;
	
#if WITH_EDITOR
	// Spawner 복제 시, GUid 복사 방지
	virtual void PostEditImport() override;
#endif
	
private:
	// 전부 스폰
	void SpawnAllSlots();
	
	// 개별 스폰
	void SpawnSlot(int32 SlotIndex);
	
	// 스폰될 채집물 선택
	const FCPResourceSpawnEntry* SelectResource(int32 SlotIndex, int32 Generation) const;
	
	// 키 생성
	FCPResourceNodeKey MakeNodeKey(int32 SlotIndex) const;
	
	// 스폰 위치 / 회전 계산
	FTransform CalculateSpawnTransform(int32 SlotIndex, int32 Generation) const;
	
	// 스폰 시드 생성
	int32 MakeSpawnSeed(int32 SlotIndex, int32 Generation, int32 Salt) const;
	
	// 현재 스포너가 속한 레벨 Id 가져오기
	FName GetLevelId() const;

private:	
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<USceneComponent> SceneRoot;
	
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UBoxComponent> SpawnArea;
	
	// 스폰될 채집물 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Resource")
	TSubclassOf<ACPResourceNodeActor> ResourceNodeActorClass;
	
	UPROPERTY(EditAnywhere, Category = "Resource")
	TArray<FCPResourceSpawnEntry> SpawnEntries;
	
	// 스폰 수
	UPROPERTY(EditAnywhere, Category = "Resource", meta = (ClampMin = "1"))
	int32 SpawnCount = 1;
	
	// 스포너 Id
	UPROPERTY(VisibleAnywhere, Category = "Resource")
	FGuid SpawnerId;
	
	// 최대 허용 경사각(기본적으로 모든 채집물은 하늘을 향하고, 바닥 경사면의 각도에 따라 기울어짐. 기울어지는 정도의 최대치)
	UPROPERTY(EditAnywhere, Category = "Resource")
	float MaxGroundAlignAngle = 15;
};
