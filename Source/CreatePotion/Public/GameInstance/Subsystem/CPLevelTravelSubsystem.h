// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "CPLevelTravelSubsystem.generated.h"

/**
 * 
 */
class UWorld;
class APlayerStart;

UCLASS()
class CREATEPOTION_API UCPLevelTravelSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Level Travel")
	void TravelToLevel(TSoftObjectPtr<UWorld> TargetLevel, FName DestinationSpawnPointId);

	UFUNCTION(BlueprintPure, Category = "Level Travel")
	FName GetPendingSpawnPointId() const;
	
	void ConsumePendingSpawnPointId();

	UPROPERTY(BlueprintReadOnly, Category = "Level Travel")
	FName PreviousLevelName = NAME_None;
	
private:
	UPROPERTY()
	FName PendingSpawnPointId = NAME_None;

};
