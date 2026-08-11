#include "NPC/CPNPCSpawner.h"
#include "NPC/CPBaseNPC.h"
#include "Data/CPNPCDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/QuestManager.h"

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

	// 수락된 퀘스트 순서대로 NPCSpawnConfigArray에서 알맞은 NPC 설정 필터링
	for (const FName& QuestID : AcceptedQuests)
	{
		for (const FNPCSpawnConfig& Config : NPCSpawnConfigArray)
		{
			if (Config.NPCData && Config.NPCData->LabQuestIDs.Contains(QuestID))
			{
				FilteredNPCsToSpawn.Add(Config);
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

	const FNPCSpawnConfig& Config = FilteredNPCsToSpawn[CurrentSpawnIndex];

	if (Config.NPCData)
	{
		// 개별 트랜스폼 체크 시 CustomTransform, 미체크 시 DefaultSpawnTransform 적용
		FTransform FinalTransform = Config.bUseCustomTransform ? Config.CustomTransform : DefaultSpawnTransform;

		ACPBaseNPC* SpawnedNPC = GetWorld()->SpawnActorDeferred<ACPBaseNPC>(NPCClass, FinalTransform);
		if (SpawnedNPC)
		{
			SpawnedNPC->NPCData = Config.NPCData;
			UGameplayStatics::FinishSpawningActor(SpawnedNPC, FinalTransform);
		}
	}

	CurrentSpawnIndex++;

	if (CurrentSpawnIndex >= FilteredNPCsToSpawn.Num())
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}
}