#pragma once

#include "CoreMinimal.h"
#include "GameCore/Interface/CPPoolable.h"
#include "GameFramework/Actor.h"
#include "Resource/CPResourceType.h"
#include "CPResourceNodeActor.generated.h"

class UCPHarvestComponent;
class UCPResourceDefinition;

UCLASS()
class CREATEPOTION_API ACPResourceNodeActor : public AActor, public ICPPoolable
{
	GENERATED_BODY()
	
public:	
	ACPResourceNodeActor();
	
	// 초기화
	void InitializeResource(const FCPResourceNodeKey& InNodeKey, UCPResourceDefinition* InDefinition, int32 InHarvestMultiplier);
	
	// 오브젝트 풀링 관련
	virtual void OnAcquireFromPool_Implementation() override;
	
	virtual void OnReleaseToPool_Implementation() override;
	
private:
	// 채집물에 해당하는 정보 적용
	void ApplyDefinition();

private:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(VisibleAnywhere, Category =  "Component")
	TObjectPtr<UCPHarvestComponent> HarvestComponent;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Resource")
	FCPResourceNodeKey NodeKey;
	
	UPROPERTY(Transient)
	TObjectPtr<UCPResourceDefinition> ResourceDefinition;
	
	int32 HarvestMultiplier = 1;
};
