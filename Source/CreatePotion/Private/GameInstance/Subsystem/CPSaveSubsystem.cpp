#include "GameInstance/Subsystem/CPSaveSubsystem.h"
#include "Data/CPSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Resource/System/CPResourceStateSubsystem.h"

void UCPSaveSubsystem::SaveGame()
{
	UCPSaveGame* SaveData = Cast<UCPSaveGame>(UGameplayStatics::CreateSaveGameObject(UCPSaveGame::StaticClass()));
	if (!SaveData) return;
	
	UGameInstance* GI = GetGameInstance();
	
	//TODO: 각 서브시스템에서 데이터 수집
	
	UGameplayStatics::SaveGameToSlot(SaveData, SlotName, 0);
}

void UCPSaveSubsystem::LoadGame()
{
	UCPSaveGame* SaveData = Cast<UCPSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!SaveData) return;
	
	UGameInstance* GI = GetGameInstance();
	
	//TODO: 각 서브시스템으로 데이터 로드.
	//서브시스템 내부에서 받은 데이터를 로드하도록 구현
}
