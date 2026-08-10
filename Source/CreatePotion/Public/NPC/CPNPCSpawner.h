#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPNPCSpawner.generated.h"

class ACPBaseNPC;
class UCPNPCDataAsset;

UCLASS()
class CREATEPOTION_API ACPNPCSpawner : public AActor
{
	GENERATED_BODY()

public:
	ACPNPCSpawner();

	// 벨을 울릴 때 호출하여 스폰 세션을 시작하는 함수
	UFUNCTION(BlueprintCallable, Category = "NPC Spawner")
	void StartSpawningSession();

protected:
	virtual void BeginPlay() override;

private:
	void SpawnNextNPC();

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	TSubclassOf<ACPBaseNPC> NPCClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TArray<TObjectPtr<UCPNPCDataAsset>> NPCDataArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Spawn")
	TArray<FTransform> SpawnTransforms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Spawn")
	float SpawnInterval = 2.0f;

private:
	FTimerHandle SpawnTimerHandle;
	int32 CurrentSpawnIndex = 0;

	UPROPERTY()
	TArray<UCPNPCDataAsset*> FilteredNPCsToSpawn;

};