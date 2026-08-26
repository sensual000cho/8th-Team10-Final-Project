#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Resource/CPResourceType.h"
#include "CPFarmCropDefinition.generated.h"

// 농작물 DA
UCLASS()
class CREATEPOTION_API UCPFarmCropDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 식별용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName CropId;
	
	// 성장 완료까지 걸리는 시간(게임 시간)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1"))
	float GrowthDuration = 1440.f;
	
	// 성장 중간 단계 진입 시점
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float IntermediateStageRatio = 0.5f;
	
	// 성장 단계별로 메시 다르게
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TSoftObjectPtr<UStaticMesh> SeedMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TSoftObjectPtr<UStaticMesh> IntermediateMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TSoftObjectPtr<UStaticMesh> MatureMesh;
	
	// 채집 데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	FCPHarvestData HarvestData;
};
