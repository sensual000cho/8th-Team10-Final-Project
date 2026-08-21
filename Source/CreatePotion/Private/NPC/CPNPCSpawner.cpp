#include "NPC/CPNPCSpawner.h"
#include "NPC/CPBaseNPC.h"
#include "Data/CPNPCDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/QuestManager.h"
#include "GameMode/CPLabGameMode.h"
#include "Lab/CPLabTypes.h"

ACPNPCSpawner::ACPNPCSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool ACPNPCSpawner::SpawnNPC(FName QuestID)
{
	if (!NPCClass || QuestID.IsNone()) return false;

	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;

	if (QuestManager && QuestManager->GetQuestState(QuestID) != EQuestState::Accepted)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CPNPCSpawner] 수락되지 않은 퀘스트라 스폰을 취소합니다: %s"), *QuestID.ToString());
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

	if (!FoundConfig || !FoundConfig->NPCData) return false;

	const FTransform FinalTransform = FoundConfig->bUseCustomTransform ? FoundConfig->CustomTransform : DefaultSpawnTransform;

	ACPBaseNPC* SpawnedNPC = GetWorld()->SpawnActorDeferred<ACPBaseNPC>(NPCClass, FinalTransform);
	if (SpawnedNPC)
	{
		SpawnedNPC->NPCData = FoundConfig->NPCData;
		UGameplayStatics::FinishSpawningActor(SpawnedNPC, FinalTransform);
	}

	if (UWorld* World = GetWorld())
	{
		if (ACPLabGameMode* LabGameMode = World->GetAuthGameMode<ACPLabGameMode>())
		{
			LabGameMode->StartPotionRequest(QuestID);
		}
	}
	
	return true;
}