#pragma once

#include "CoreMinimal.h"
#include "GameCore/Interface/CPInteractable.h"
#include "GameFramework/Actor.h"
#include "CPDroppedItemBase.generated.h"

class UCPForageableItemData;

// 바닥에 떨구는 아이템 베이스 액터
UCLASS()
class CREATEPOTION_API ACPDroppedItemBase : public AActor, public ICPInteractable
{
	GENERATED_BODY()
	
public:	
	ACPDroppedItemBase();
	
	// 상호작용 인터페이스 관련
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	
	virtual FText GetInteractionPrompt_Implementation() override;
	
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	
	// 아이템 데이터 초기화
	void Initialize(UCPForageableItemData* InItemData, int32 InAmount, UStaticMesh* InMesh);
	
protected:
	// 현재 아이템 데이터가 재료밖에 없어서 재료 아이템 데이터로 함
	UPROPERTY(Transient)
	TObjectPtr<UCPForageableItemData> ItemData;
	
	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	int32 Amount = 1;
};
