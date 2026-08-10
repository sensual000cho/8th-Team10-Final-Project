#include "NPC/CPNPCSpawner.h"
#include "NPC/CPBaseNPC.h"
#include "Data/CPNPCDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/QuestManager.h"
#include "Math/UnrealMathUtility.h" 

ACPNPCSpawner::ACPNPCSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}
void ACPNPCSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void ACPNPCSpawner::StartSpawningSession()
{
	if (!NPCClass) return;

	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;

	if (!QuestManager) return;

	FilteredNPCsToSpawn.Empty();
	CurrentSpawnIndex = 0;

	const TArray<FName>& AcceptedQuests = QuestManager->AcceptedQuestOrder;

	for (const FName& QuestID : AcceptedQuests)
	{
		for (UCPNPCDataAsset* Data : NPCDataArray)
		{
			if (Data && Data->LabQuestIDs.Contains(QuestID))
			{
				FilteredNPCsToSpawn.Add(Data);
				break;
			}
		}
	}

	if (FilteredNPCsToSpawn.Num() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&ACPNPCSpawner::SpawnNextNPC,
			SpawnInterval,
			true,
			0.0f
		);
	}
}

void ACPNPCSpawner::SpawnNextNPC()
{
	if (CurrentSpawnIndex >= FilteredNPCsToSpawn.Num())
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	UCPNPCDataAsset* Data = FilteredNPCsToSpawn[CurrentSpawnIndex];

	if (Data && SpawnTransforms.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, SpawnTransforms.Num() - 1);
		FTransform RandomTransform = SpawnTransforms[RandomIndex];

		ACPBaseNPC* SpawnedNPC = GetWorld()->SpawnActorDeferred<ACPBaseNPC>(NPCClass, RandomTransform);
		if (SpawnedNPC)
		{
			SpawnedNPC->NPCData = Data;
			UGameplayStatics::FinishSpawningActor(SpawnedNPC, RandomTransform);
		}
	}

	CurrentSpawnIndex++;

	if (CurrentSpawnIndex >= FilteredNPCsToSpawn.Num())
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}
}