#include "NPC/CPNPCSpawner.h"
#include "NPC/CPBaseNPC.h"
#include "Data/NPC/CPQuestNPCDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/QuestManager.h"

ACPNPCSpawner::ACPNPCSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool ACPNPCSpawner::SpawnNPC(FName QuestID)
{
	if (!NPCClass || QuestID.IsNone())
	{
		return false;
	}

	if (ActiveQuestNPCs.Contains(QuestID) && IsValid(ActiveQuestNPCs[QuestID]))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CPNPCSpawner] 이미 스폰되었습니다 %s"), *QuestID.ToString());
		return false;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;

	if (QuestManager && QuestManager->GetQuestState(QuestID) != EQuestState::Accepted)
	{
		return false;
	}

	const FNPCSpawnConfig* FoundConfig = nullptr;
	for (const FNPCSpawnConfig& Config : NPCSpawnConfigArray)
	{
		if (Config.NPCData && Config.NPCData->LabQuestIDs.Contains(QuestID))
		{
			FoundConfig = &Config;
			break;
		}
	}

	if (!FoundConfig || !FoundConfig->NPCData)
	{
		return false;
	}

	const FTransform FinalTransform = FoundConfig->bUseCustomTransform ? FoundConfig->CustomTransform : DefaultSpawnTransform;

	ACPBaseNPC* SpawnedNPC = GetWorld()->SpawnActorDeferred<ACPBaseNPC>(NPCClass, FinalTransform);
	if (SpawnedNPC)
	{
		SpawnedNPC->NPCData = FoundConfig->NPCData;
		UGameplayStatics::FinishSpawningActor(SpawnedNPC, FinalTransform);
		ActiveQuestNPCs.Add(QuestID, SpawnedNPC);
	}

	return true;
}