#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPNPCSpawner.generated.h"

class ACPBaseNPC;
class UCPNPCDataAsset;
class UQuestManager;

// DA와 개별 스폰 트랜스폼을 액터에서 설정하기 위한 구조체
USTRUCT(BlueprintType)
struct FNPCSpawnConfig
{
	GENERATED_BODY()

	// 스폰할 NPC의 Data Asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TObjectPtr<UCPNPCDataAsset> NPCData = nullptr;

	// 이 NPC에 개별 트랜스폼을 적용할지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	bool bUseCustomTransform = false;

	// 개별 트랜스폼 (bUseCustomTransform가 true일 때만 디테일 패널에 활성화됨)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC", meta = (EditCondition = "bUseCustomTransform"))
	FTransform CustomTransform;
};

UCLASS()
class CREATEPOTION_API ACPNPCSpawner : public AActor
{
	GENERATED_BODY()

public:
	ACPNPCSpawner();

	UFUNCTION(BlueprintCallable, Category = "NPC Spawner")
	void StartSpawningSession();

	// 현재 Accepted 상태인 Lab 퀘스트 ID들만 추려서 반환 (포션 리퀘스트 생성용)
	TArray<FName> GetAcceptedLabQuestIDs(const UQuestManager* QuestManager) const;

protected:
	virtual void BeginPlay() override;

private:
	void SpawnNextNPC();

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	TSubclassOf<ACPBaseNPC> NPCClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Spawn")
	FTransform DefaultSpawnTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Spawn")
	TArray<FNPCSpawnConfig> NPCSpawnConfigArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Spawn")
	float SpawnInterval = 2.0f;

private:
	FTimerHandle SpawnTimerHandle;
	int32 CurrentSpawnIndex = 0;

	UPROPERTY()
	TArray<FNPCSpawnConfig> FilteredNPCsToSpawn;

};