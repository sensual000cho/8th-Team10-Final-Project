// CPContainerSubsystem.cpp

#include "GameInstance/Subsystem/CPContainerSubsystem.h"

#include "CreatePotion.h"				// 로그용 헤더
#include "Types/CPContainerTypes.h"		// 아이템 구조체

void UCPContainerSubsystem::SaveContainerData(const TArray<FContainerItem>& InItems)
{
	BackupContainerItems = InItems;
	bHasBackup = true;
	UE_LOG(LogContainer, Log, TEXT("[ContainerSubsystem] 컨테이너 데이터 %d개 백업 완료"),
		BackupContainerItems.Num());
}

bool UCPContainerSubsystem::LoadContainerData(TArray<FContainerItem>& OutItems)
{
	if (bHasBackup)
	{
		OutItems = BackupContainerItems;
		UE_LOG(LogContainer, Log, TEXT("[ContainerSubsystem] 컨테이너 데이터 복원 완료"));
		return true;
	}
	return false;
}
