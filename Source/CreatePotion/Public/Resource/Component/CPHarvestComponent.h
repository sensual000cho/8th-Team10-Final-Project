#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPHarvestComponent.generated.h"


class ACPDroppedItemBase;
class UCPForageableItemData;

// 채집물 / 농작물용 수확 컴포넌트
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPHarvestComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCPHarvestComponent();

	void Initialize(
		UCPForageableItemData* InItemData,
		int32 InAmount,
		UStaticMesh* InDroppedItemMesh,
		TSubclassOf<ACPDroppedItemBase> InDroppedItemClass);
	
	bool TryHarvest(AActor* Interactor);
	
private:
	UPROPERTY(Transient)
	TObjectPtr<UCPForageableItemData> ItemData;
	
	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> DroppedItemMesh;
	
	UPROPERTY(Transient)
	TSubclassOf<ACPDroppedItemBase> DroppedItemClass;
	
	int32 Amount = 0;
};
