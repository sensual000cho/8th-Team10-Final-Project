#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NPC/CPNPCTypes.h"
#include "CPNPCDataAsset.generated.h"

class USkeletalMesh;

UCLASS()
class CREATEPOTION_API UCPNPCDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "NPC")
	FName NPCName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	TSoftObjectPtr<USkeletalMesh> NPCMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	FTransform SpawnTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Capsule")
	float CapsuleRadiusRatio = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TMap<ECPNPCSituation, FCPNPCSituationData> SituationData;
};