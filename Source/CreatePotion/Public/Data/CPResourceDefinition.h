#pragma once

#include "CoreMinimal.h"
#include "CPForageableItemData.h"
#include "NiagaraSystem.h"
#include "Engine/DataAsset.h"
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TSoftObjectPtr<UNiagaraSystem> IdleVFX;
	
	// Harvest VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TSoftObjectPtr<UNiagaraSystem> HarvestVFX;
	
	// 채집 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	float HarvestDuration = 1.f;
	
	// 채집 아이템
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	TSoftObjectPtr<UCPForageableItemData> HarvestedItem;
	
	// 채집 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest",
		meta = (ClampMin = "1"))
	int32 HarvestAmount = 1;
	
	// 리스폰 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn",
		meta = (ClampMin = "10"))
	float RespawnDuration = 60.f;
};
