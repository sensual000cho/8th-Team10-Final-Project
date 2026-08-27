// CPContainerSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CPContainerSubsystem.generated.h"

struct FContainerItem;

UCLASS()
class CREATEPOTION_API UCPContainerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void SaveContainerData(const TArray<FContainerItem>& InItems);

	bool LoadContainerData(TArray<FContainerItem>& OutItems);

	// 레벨 전환 시 임시로 보관할 인벤토리 배열
	UPROPERTY()
	TArray<FContainerItem> BackupContainerItems;

	// 백업된 데이터가 있는지 여부 (처음 게임 시작 시엔 안 부르기 위함)
	UPROPERTY()
	bool bHasBackup = false;
};

