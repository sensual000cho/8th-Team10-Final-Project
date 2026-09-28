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
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveGame();
	
	UFUNCTION(BlueprintCallable, Category = "Save")	
	void LoadGame();
	
	void ApplyLoadPlayerTransform(APawn* InPawn);
	
private:
	// 세이브 슬롯
	FString SlotName = TEXT("Slot1");
	
	bool bHasLoadedTransform = false;
	
	FVector LoadedLocation = FVector::ZeroVector;
	
	FRotator LoadedRotation = FRotator::ZeroRotator;
};
