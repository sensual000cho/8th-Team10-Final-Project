#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CPNPCTargetData.generated.h"

USTRUCT(BlueprintType)
struct FCPTargetPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Point = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaitSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct FCPNPCRoute
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TSoftObjectPtr<UWorld> TargetLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	TArray<FCPTargetPoint> TargetPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bLoop = true;
};

USTRUCT(BlueprintType)
struct FCPNPCTargetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCPNPCRoute> Routes;
};