// CPEnemySpawner.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPEnemySpawner.generated.h"

class ACPEnemyCharacter;

UCLASS()
class CREATEPOTION_API ACPEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ACPEnemySpawner();

protected:
	virtual void BeginPlay() override;

private:
	void SpawnEnemies();
	bool TryFindSpawnLocation(FVector& OutLocation);
	ACPEnemyCharacter* SpawnEnemyAtLocation(const FVector& SpawnLocation);
	
	// 이 지역에서 생성할 슬라임 종류
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<ACPEnemyCharacter> EnemyClass;

	// 생성할 마릿수
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = "0"))
	int32 SpawnCount = 5;

	// 최초 스폰 위치를 선정할 영역(슬라임의 영역에 따라 다르게)
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = "0.0", Units = "cm"))
	float SpawnRadius = 10000.f;

	// 스폰 시 다른 슬라임과의 최소 거리
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = "0.0", Units = "cm"))
	float MinSpawnSpacing = 1000.f;

	// 스폰위치 찾기 최대 10번 시도
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = "1"))
	int32 MaxSpawnAttemptsPerEnemy = 10;
	
	// 스폰에 성공한 슬라임들의 최초 위치
	UPROPERTY(VisibleInstanceOnly, Category = "Spawner")
	TArray<FVector> SpawnedLocations;
	
	// 바닥에 파묻히지 않게
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = "0.0", Units = "cm"))
	float SpawnHeightOffset = 5.f;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<ACPEnemyCharacter>> SpawnedEnemies;
};
