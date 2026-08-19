#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CPSaveSubsystem.generated.h"

// 세이브 / 로드 담당 서브시스템
UCLASS()
class CREATEPOTION_API UCPSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void SaveGame();
	void LoadGame();
	
private:
	// 세이브 슬롯
	FString SlotName = TEXT("Slot1");
};
