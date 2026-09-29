#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameInstance/Subsystem/CPCauldronSubsystem.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "GameInstance/Subsystem/CPContainerSubsystem.h"
#include "GameInstance/Subsystem/CPNPCSubsystem.h"
#include "Quest/QuestManager.h"
#include "Resource/CPResourceType.h"
#include "CPSaveGame.generated.h"

// 저장되어야 할 데이터들 모음
UCLASS()
class CREATEPOTION_API UCPSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	// 시간 상태 정보
	UPROPERTY()
	int64 TotalWorldMinutes = 0;
	
	// 인벤토리 상태 정보
	UPROPERTY()
	FCPInventorySaveData InventoryData;
	
	// 채집물 상태 정보
	UPROPERTY()
	FCPResourceSaveData ResourceData;
	
	// 도감 상태 정보
	UPROPERTY()
	FCPCodexSaveData CodexData;
	
	// NPC 상태 정보
	UPROPERTY()
	FCPNPCSaveData NPCData;
	
	// 퀘스트 상태 정보
	//UPROPERTY()
	//FCPQuestSaveData QuestData;
	
	// 가마솥 상태 정보
	UPROPERTY()
	FCPCauldronSaveData CauldronData;
	
	UPROPERTY()
	FName LevelName;
	
	UPROPERTY()
	FVector PlayerLocation = FVector::ZeroVector;
	
	UPROPERTY()
	FRotator PlayerRotation = FRotator::ZeroRotator;
};
