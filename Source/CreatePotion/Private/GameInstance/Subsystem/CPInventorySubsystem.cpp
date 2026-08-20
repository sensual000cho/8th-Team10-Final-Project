// CPInventorySubsystem.cpp

#include "GameInstance/Subsystem/CPInventorySubsystem.h"

#include "CreatePotion.h"		// 로그용 헤더
#include "Types/CPContainerTypes.h"		// 아이템 구조체

void UCPInventorySubsystem::SaveInventoryData(const TArray<FContainerItem>& InItems)
{
	BackupInventoryItems = InItems;
	bHasBackup = true;
	UE_LOG(LogContainer, Log, TEXT("[InventorySubsystem] 인벤토리 데이터 %d개 백업 완료"), 
		BackupInventoryItems.Num());
}

bool UCPInventorySubsystem::LoadInventoryData(TArray<FContainerItem>& OutItems)
{
	if (bHasBackup)
	{
		OutItems = BackupInventoryItems;
		UE_LOG(LogContainer, Log, TEXT("[InventorySubsystem] 인벤토리 데이터 복원 완료"));
		return true;
	}
	return false;
}
