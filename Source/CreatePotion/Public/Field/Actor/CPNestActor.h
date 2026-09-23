#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Field/Actor/CPBaseFieldActor.h"
#include "CPNestActor.generated.h"

class ACPDroppedItemBase;
class UCPResourceDefinition;

// 필드 레벨에 배치 될 둥지 액터. 색에 맞는 포션에 반응해서 재료 아이템 드랍
UCLASS()
class CREATEPOTION_API ACPNestActor : public ACPBaseFieldActor
{
	GENERATED_BODY()
	
public:
	ACPNestActor();
	
	virtual bool PotionReaction(const FGameplayTag& EventTag, AActor* InstigatorActor = nullptr) override;
	
protected:
	// 드랍 아이템 DA
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop")
	TObjectPtr<UCPResourceDefinition> DropItemData;
	
	// 드랍 아이템 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop")
	TSubclassOf<ACPDroppedItemBase> DroppedItemClass;
	
	// 나무 위 끼임 방지용 드랍 위치 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop")
	TObjectPtr<USceneComponent> DropPoint;
};
