#include "NPC/CPNPCSpawner.h"
#include "NPC/CPBaseNPC.h"
#include "NPC/CPNPCDataAsset.h"
#include "Kismet/GameplayStatics.h"

ACPNPCSpawner::ACPNPCSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACPNPCSpawner::BeginPlay()
{
	Super::BeginPlay();
	SpawnNPC();
}

void ACPNPCSpawner::SpawnNPC()
{
    if (!NPCClass) return;

    for (UCPNPCDataAsset* Data : NPCDataArray)
    {
        if (!Data) continue;

        FTransform SpawnTransform = Data->SpawnTransform;
        ACPBaseNPC* SpawnedNPC = GetWorld()->SpawnActorDeferred<ACPBaseNPC>(NPCClass, SpawnTransform);
        if (!SpawnedNPC) continue;

        SpawnedNPC->NPCData = Data;
        UGameplayStatics::FinishSpawningActor(SpawnedNPC, SpawnTransform);
    }
}