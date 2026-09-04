// CPContainerSubsystem.cpp

#include "GameInstance/Subsystem/CPContainerSubsystem.h"

#include "CreatePotion.h"				// 로그용 헤더
#include "Types/CPContainerTypes.h"		// 아이템 구조체
#include "Components/CPInventoryComponent.h"

void UCPContainerSubsystem::SaveContainerData(UCPInventoryComponent* PlayerInventory)
{
	if (!PlayerInventory)
	{
		UE_LOG(LogContainer, Warning, TEXT("[ContainerSubsystem] 저장 실패: 인벤토리가 없습니다."));
		return;
	}

	SaveContainerItem(PlayerInventory->ContainerItems);
	SavePlayerMoney(PlayerInventory->GetOwningMoney());

	bHasBackup = true;
	UE_LOG(LogContainer, Log, TEXT("[ContainerSubsystem] 전체 데이터 저장 완료"));
}

bool UCPContainerSubsystem::LoadContainerData(UCPInventoryComponent* PlayerInventory)
{
	if (!PlayerInventory || !bHasBackup)
	{
		return false;
	}

	TArray<FContainerItem> LoadedItems;
	int32 LoadedMoney = 0;

	bool bItemsLoaded = LoadContainerItem(LoadedItems);
	bool bMoneyLoaded = LoadPlayerMoney(LoadedMoney);

	if (bItemsLoaded)
	{
		PlayerInventory->ContainerItems = LoadedItems;
		PlayerInventory->OnContainerUpdated.Broadcast(); // UI 갱신
	}

	if (bMoneyLoaded)
	{
		// TrySpendMoney/TryGetMoney를 거치지 않고 직접 복원하는 전용 함수가 필요함 (아래 참고)
		PlayerInventory->RestoreMoney(LoadedMoney);
	}

	UE_LOG(LogContainer, Log, TEXT("[ContainerSubsystem] 전체 데이터 복원 완료"));
	return true;
}


void UCPContainerSubsystem::SaveContainerItem(const TArray<FContainerItem>& InItems)
{
	BackupContainerItems = InItems;
}

bool UCPContainerSubsystem::LoadContainerItem(TArray<FContainerItem>& OutItems)
{
	OutItems = BackupContainerItems;
	return true;
}

void UCPContainerSubsystem::SavePlayerMoney(const int32& InMoney)
{
	BackupPlayerMoney = InMoney;
}

bool UCPContainerSubsystem::LoadPlayerMoney(int32& OutMoney)
{
	OutMoney = BackupPlayerMoney;
	return true;
}
