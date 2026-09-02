// CPContainerSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CPContainerSubsystem.generated.h"

struct FContainerItem;
class UCPInventoryComponent;

UCLASS()
class CREATEPOTION_API UCPContainerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Container|Save")
	void SaveContainerData(UCPInventoryComponent* PlayerInventory);

	UFUNCTION(BlueprintCallable, Category = "Container|Save")
	bool LoadContainerData(UCPInventoryComponent* PlayerInventory);

private:
	void SaveContainerItem(const TArray<FContainerItem>& InItems);

	bool LoadContainerItem(TArray<FContainerItem>& OutItems);

	void SavePlayerMoney(const int32& InMoney);

	bool LoadPlayerMoney(int32& OutMoney);

public:
	// 레벨 전환 시 임시로 보관할 인벤토리 배열
	UPROPERTY()
	TArray<FContainerItem> BackupContainerItems;

	UPROPERTY()
	int32 BackupPlayerMoney = 0;

	// 백업된 데이터가 있는지 여부 (처음 게임 시작 시엔 안 부르기 위함)
	UPROPERTY()
	bool bHasBackup = false;
};

