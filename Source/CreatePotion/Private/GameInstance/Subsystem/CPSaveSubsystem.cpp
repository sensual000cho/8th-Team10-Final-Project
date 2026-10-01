#include "GameInstance/Subsystem/CPSaveSubsystem.h"

#include "Data/CPSaveGame.h"
#include "GameInstance/Subsystem/CPLevelTravelSubsystem.h"
#include "GameInstance/Subsystem/CPTimeSubsystem.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Resource/System/CPResourceStateSubsystem.h"

void UCPSaveSubsystem::SaveGame()
{
	UCPSaveGame* SaveData = Cast<UCPSaveGame>(UGameplayStatics::CreateSaveGameObject(UCPSaveGame::StaticClass()));
	if (!SaveData) return;
	
	UGameInstance* GI = GetGameInstance();

	if (UCPTimeSubsystem* TimeSubsystem = GI->GetSubsystem<UCPTimeSubsystem>()){
		SaveData->TotalWorldMinutes = TimeSubsystem->GetTotalWorldMinutes();
	}
	
	if (UCPContainerSubsystem* ContainerSubsystem = GI->GetSubsystem<UCPContainerSubsystem>()){
		SaveData->InventoryData = ContainerSubsystem->GetSaveData();	
	}
	
	if (UCPResourceStateSubsystem* ResourceSubsystem = GI->GetSubsystem<UCPResourceStateSubsystem>()){
		SaveData->ResourceData = ResourceSubsystem->GetSaveData();
	}
	
	if (UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>()){
		SaveData->CodexData = CodexSubsystem->GetSaveData();
	}
	
	if (UCPNPCSubsystem* NPCSubsystem = GI->GetSubsystem<UCPNPCSubsystem>()){
		SaveData->NPCData = NPCSubsystem->GetSaveData();
	}
	
	if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>()){
		SaveData->QuestData = QuestManager->GetSaveData();
	}
	
	if (UCPCauldronSubsystem* CauldronSubsystem = GI->GetSubsystem<UCPCauldronSubsystem>()){
		SaveData->CauldronData = CauldronSubsystem->GetSaveData();
	}
	
	// 현재 레벨, 플레이어 위치, 방향 저장
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)){
		SaveData->LevelName = FName(*UGameplayStatics::GetCurrentLevelName(GetWorld(), true));
		SaveData->PlayerLocation = PlayerPawn->GetActorLocation();
		SaveData->PlayerRotation = PlayerPawn->GetActorRotation();
	}
	
	UGameplayStatics::SaveGameToSlot(SaveData, SlotName, 0);
}

void UCPSaveSubsystem::LoadGame()
{
	UCPSaveGame* SaveData = Cast<UCPSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!SaveData) return;
	
	UGameInstance* GI = GetGameInstance();
	
	if (UCPTimeSubsystem* TimeSubsystem = GI->GetSubsystem<UCPTimeSubsystem>()){
		TimeSubsystem->LoadSaveData(SaveData->TotalWorldMinutes);
	}
	
	if (UCPContainerSubsystem* ContainerSubsystem = GI->GetSubsystem<UCPContainerSubsystem>()){
		ContainerSubsystem->LoadSaveData(SaveData->InventoryData);	
	}
	
	if (UCPResourceStateSubsystem* ResourceSubsystem = GI->GetSubsystem<UCPResourceStateSubsystem>()){
		ResourceSubsystem->LoadSaveData(SaveData->ResourceData);
	}
	
	if (UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>()){
		CodexSubsystem->LoadSaveData(SaveData->CodexData);
	}
	
	if (UCPNPCSubsystem* NPCSubsystem = GI->GetSubsystem<UCPNPCSubsystem>()){
		NPCSubsystem->LoadSaveData(SaveData->NPCData);
	}
	
	if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>()){
		QuestManager->LoadSaveData(SaveData->QuestData);
	}
	
	if (UCPCauldronSubsystem* CauldronSubsystem = GI->GetSubsystem<UCPCauldronSubsystem>()){
		CauldronSubsystem->LoadSaveData(SaveData->CauldronData);
	}
	
	LoadedLocation = SaveData->PlayerLocation;
	LoadedRotation = SaveData->PlayerRotation;
	bHasLoadedTransform = true;
	
	UGameplayStatics::OpenLevel(GetWorld(), SaveData->LevelName);
}

void UCPSaveSubsystem::ResetGameSession()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	bHasLoadedTransform = false;
	LoadedLocation = FVector::ZeroVector;
	LoadedRotation = FRotator::ZeroRotator;
	
	if (UCPContainerSubsystem* ContainerSubsystem = GI->GetSubsystem<UCPContainerSubsystem>()){
		ContainerSubsystem->ResetData();
	}
	
	if (UCPResourceStateSubsystem* ResourceSubsystem = GI->GetSubsystem<UCPResourceStateSubsystem>()){
		ResourceSubsystem->ResetData();
	}

	if (UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>()){
		CodexSubsystem->ResetData();
	}
	
	if (UCPNPCSubsystem* NPCSubsystem = GI->GetSubsystem<UCPNPCSubsystem>()){
		NPCSubsystem->ResetData();
	}
	
	if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>()){
		QuestManager->ResetData();
	}
	
	if (UCPCauldronSubsystem* CauldronSubsystem = GI->GetSubsystem<UCPCauldronSubsystem>()){
		CauldronSubsystem->ResetData();
	} 
	
	if (UCPTimeSubsystem* TimeSubsystem = GI->GetSubsystem<UCPTimeSubsystem>()){
		TimeSubsystem->ResetData();
	}
}

void UCPSaveSubsystem::ApplyLoadPlayerTransform(APawn* InPawn)
{
	if (!bHasLoadedTransform || !InPawn) return;
	
	InPawn->SetActorLocationAndRotation(LoadedLocation, LoadedRotation);
	
	bHasLoadedTransform = false;
}
