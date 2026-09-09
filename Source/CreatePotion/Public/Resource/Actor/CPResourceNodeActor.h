#pragma once

#include "CoreMinimal.h"
#include "GameCore/Interface/CPPoolable.h"
#include "GameFramework/Actor.h"
#include "Items/CPBaseInteractableActor.h"
#include "Resource/CPResourceType.h"
#include "CPResourceNodeActor.generated.h"

class UCPHarvestComponent;
class UCPResourceDefinition;
class UCPInteractionComponent;
class UCPInteractableComponent;
class ACPDroppedItemBase;
class UAnimMontage;
class UWidgetComponent;

UCLASS()
class CREATEPOTION_API ACPResourceNodeActor : public ACPBaseInteractableActor, public ICPPoolable
{
	GENERATED_BODY()
	
public:	
	ACPResourceNodeActor();

	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// 초기화
	void InitializeResource(const FCPResourceNodeKey& InNodeKey, UCPResourceDefinition* InDefinition);
	
	// 인터랙션 델리게이트 콜백 함수
	void HandleInteract(AActor* Interactor) override;
	void HandleInteractionStarted(AActor* Interactor) override;
	
	// 오브젝트 풀링 관련
	virtual void OnAcquireFromPool_Implementation() override;
	
	virtual void OnReleaseToPool_Implementation() override;
	
private:
	// 채집물에 해당하는 정보 적용
	void ApplyDefinition();
	
	// 채집
	void Harvest(AActor* Interactor);
	
	// 조사
	void Inspect(AActor* Interactor);
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(VisibleAnywhere, Category =  "Component")
	TObjectPtr<UCPHarvestComponent> HarvestComponent;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Resource")
	FCPResourceNodeKey NodeKey;
	
	UPROPERTY(EditDefaultsOnly, Category = "Resource")
	TSubclassOf<ACPDroppedItemBase> DroppedItemClass;
	
	UPROPERTY(Transient)
	TObjectPtr<UCPResourceDefinition> ResourceDefinition;
	
	float InspectDuration = 1.f;
	
public:
	// 채집 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource|Animation")
	TObjectPtr<UAnimMontage> HarvestMontage;
	
	// 조사 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource|Animation")
	TObjectPtr<UAnimMontage> InspectMontage;
};
