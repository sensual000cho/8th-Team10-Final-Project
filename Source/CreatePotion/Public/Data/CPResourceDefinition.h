#pragma once

#include "CoreMinimal.h"
//#include "NiagaraSystem.h"
#include "Engine/DataAsset.h"
#include "Resource/CPResourceType.h"
#include "CPResourceDefinition.generated.h"

UCLASS()
class CREATEPOTION_API UCPResourceDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 채집물 메시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TSoftObjectPtr<UStaticMesh> Mesh;
	
	// Idle VFX
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	//TSoftObjectPtr<UNiagaraSystem> IdleVFX;
	
	// Harvest VFX
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	//TSoftObjectPtr<UNiagaraSystem> HarvestVFX;
	
	// 채집 데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	FCPHarvestData HarvestData;
	
	// 리스폰 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn",
		meta = (ClampMin = "10"))
	float RespawnDuration = 60.f;
};
