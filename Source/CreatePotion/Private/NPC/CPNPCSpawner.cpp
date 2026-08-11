#include "NPC/CPNPCSpawner.h"
#include "NPC/CPBaseNPC.h"
#include "Data/CPNPCDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/QuestManager.h"
#include "GameMode/CPLabGameMode.h"

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

	const TArray<FName> AllTrackedQuests = QuestManager->GetAllTrackedQuestIDs();
	TArray<FName> AcceptedQuests;
	for (const FName& QuestID : AllTrackedQuests)
	{
		if (QuestManager->GetQuestState(QuestID) == EQuestState::Accepted)
		{
			AcceptedQuests.Add(QuestID);
		}
	}

	// 게임모드에 전달할 리퀘스트(주문서) 목록을 담을 배열 생성
	TArray<FCPLabPotionRequest> PotionRequests;

	// 수락된 퀘스트 순서대로 NPCSpawnConfigArray에서 알맞은 NPC 설정 필터링
	for (const FName& QuestID : AcceptedQuests)
	{
		for (const FNPCSpawnConfig& Config : NPCSpawnConfigArray)
		{
			if (Config.NPCData && Config.NPCData->LabQuestIDs.Contains(QuestID))
			{
				FilteredNPCsToSpawn.Add(Config);
				FCPLabPotionRequest RealRequest;
				RealRequest.RequestId = QuestID; // 진짜 퀘스트 ID (예: Origin_Q001)
				RealRequest.DisplayText = QuestManager->GetSessionHintText(QuestID);

				// 게임모드에 보낼 배열에 추가
				PotionRequests.Add(RealRequest);
				break;
			}
		}
	}
	if (FilteredNPCsToSpawn.Num() > 0)
	{
		if (ACPLabGameMode* LabMode = Cast<ACPLabGameMode>(GetWorld()->GetAuthGameMode()))
		{
			LabMode->TryStartLabSessionWithRequests(PotionRequests);
		}
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

TArray<FName> ACPNPCSpawner::GetAcceptedLabQuestIDs(const UQuestManager* QuestManager) const
{
	TArray<FName> AcceptedLabQuestIDs;
	if (!QuestManager) return AcceptedLabQuestIDs;

	// 동일하게 QuestOrder + Accepted 상태 기준으로 필터링
	const TArray<FName> AllTrackedQuests = QuestManager->GetAllTrackedQuestIDs();
	for (const FName& QuestID : AllTrackedQuests)
	{
		if (QuestManager->GetQuestState(QuestID) != EQuestState::Accepted)
		{
			continue;
		}

		for (const FNPCSpawnConfig& Config : NPCSpawnConfigArray)
		{
			if (Config.NPCData && Config.NPCData->LabQuestIDs.Contains(QuestID))
			{
				AcceptedLabQuestIDs.Add(QuestID);
				break;
			}
		}
	}

	return AcceptedLabQuestIDs;
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