// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/Subsystem/CPLevelTravelSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UCPLevelTravelSubsystem::TravelToLevel(TSoftObjectPtr<UWorld> TargetLevel, FName DestinationSpawnPointId)
{
	if (TargetLevel.IsNull())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TravelToLevel failed: 타겟 레벨 포인터 null."));

		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("TravelToLevel failed: 타겟 레벨이 없음"));

		return;
	}

	PreviousLevelName =
		FName(*UGameplayStatics::GetCurrentLevelName(World, true));

	// NAME_None도 정상적인 값으로 허용
	PendingSpawnPointId = DestinationSpawnPointId;

	UGameplayStatics::OpenLevelBySoftObjectPtr(World, TargetLevel);
}

FName UCPLevelTravelSubsystem::GetPendingSpawnPointId() const
{
	return PendingSpawnPointId;
}

void UCPLevelTravelSubsystem::ConsumePendingSpawnPointId()
{
	PendingSpawnPointId = NAME_None;
}
