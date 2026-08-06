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

protected:
	virtual void BeginPlay() override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	TSubclassOf<ACPBaseNPC> NPCClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TArray<TObjectPtr<UCPNPCDataAsset>> NPCDataArray;

private:
	void SpawnNPC();
};