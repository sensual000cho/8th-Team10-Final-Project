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

// 퀘스트 ID 입력받아 그 퀘스트의 NPC 스폰
// 퀘스트 선택 UI에서 '손님 호출' 버튼을 눌렀을 때
// 
// [적용 예시 코드]
// FName TargetQuestID = GetSelectedQuestID(); // 현재 UI에서 선택된 퀘스트 ID 가져오기
// if (!TargetQuestID.IsNone())
// {
//     // 맵에 있는 스포너를 찾아 선택한 퀘스트의 NPC를 즉시 소환
//     if (ACPNPCSpawner* Spawner = Cast<ACPNPCSpawner>(UGameplayStatics::GetActorOfClass(GetWorld(), ACPNPCSpawner::StaticClass())))
//     {
//         Spawner->SpawnNPC(TargetQuestID);
//     }
// }

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

	{
		TArray<FCPLabPotionRequest> PotionRequests;
		FCPLabPotionRequest Request;
		Request.RequestId = QuestID;

		Request.DisplayText = QuestManager->GetCurrentSessionHintText(QuestID);
		PotionRequests.Add(Request);

	}

	const FTransform FinalTransform = FoundConfig->bUseCustomTransform ? FoundConfig->CustomTransform : DefaultSpawnTransform;

	ACPBaseNPC* SpawnedNPC = GetWorld()->SpawnActorDeferred<ACPBaseNPC>(NPCClass, FinalTransform);
	if (SpawnedNPC)
	{
		SpawnedNPC->NPCData = FoundConfig->NPCData;
		UGameplayStatics::FinishSpawningActor(SpawnedNPC, FinalTransform);
	}

	// TODO : Lab게임 모드 쪽에 PotionRequests 전달
	return true;
}