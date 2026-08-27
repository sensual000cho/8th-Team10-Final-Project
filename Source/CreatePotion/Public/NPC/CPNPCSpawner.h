#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/CPNPCDataAsset.h"
#include "CPNPCSpawner.generated.h"

class ACPBaseNPC;

USTRUCT(BlueprintType)
struct FNPCSpawnConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TObjectPtr<UCPNPCDataAsset> NPCData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bUseCustomTransform = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "bUseCustomTransform"))
	FTransform CustomTransform = FTransform::Identity;
};

UCLASS()
class CREATEPOTION_API ACPNPCSpawner : public AActor
{
	GENERATED_BODY()

public:
	ACPNPCSpawner();

	// 선택한 퀘스트 ID에 해당하는 NPC 1명만 즉시 스폰
	UFUNCTION(BlueprintCallable, Category = "NPC Spawner")
	bool SpawnNPC(FName QuestID);

public:
	UPROPERTY(EditAnywhere, Category = "NPC")
	TSubclassOf<ACPBaseNPC> NPCClass;

	UPROPERTY(EditAnywhere, Category = "NPC")
	TArray<FNPCSpawnConfig> NPCSpawnConfigArray;

	UPROPERTY(EditAnywhere, Category = "NPC")
	FTransform DefaultSpawnTransform;

protected:
	UPROPERTY()
	TMap<FName, ACPBaseNPC*> ActiveQuestNPCs;

};