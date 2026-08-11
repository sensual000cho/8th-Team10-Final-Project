#pragma once

#include "CoreMinimal.h"
#include "GameCore/Interface/CPHighlightable.h"
#include "GameCore/Interface/CPInteractable.h"
#include "GameCore/Interface/CPPoolable.h"
#include "GameCore/Interface/CPTimedInteractable.h"
#include "GameFramework/Actor.h"
#include "Resource/CPResourceType.h"
#include "CPResourceNodeActor.generated.h"

class UCPResourceDefinition;
class ACPDroppedItemBase;

UCLASS()
class CREATEPOTION_API ACPResourceNodeActor : public AActor, public ICPInteractable, public ICPTimedInteractable,
											  public ICPPoolable, public ICPHighlightable
{
	GENERATED_BODY()
	
public:	
	ACPResourceNodeActor();

	// 초기화
	void InitializeResource(const FCPResourceNodeKey& InNodeKey, UCPResourceDefinition* InDefinition);
	
	// 인터랙션 인터페이스 관련
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	
	virtual FText GetInteractionPrompt_Implementation() override;
	
	virtual FName GetInteractionName_Implementation() override;
	
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	
	virtual float GetInteractionDuration_Implementation(AActor* Interactor) override;
	
	// 오브젝트 풀링 관련
	virtual void OnAcquireFromPool_Implementation() override;
	
	virtual void OnReleaseToPool_Implementation() override;
	
	// 상호작용 하이라이트 관련
	virtual void SetHighlight_Implementation(bool bHighLighted) override;
	
	UPROPERTY(EditDefaultsOnly, category = "Highlight")
	TObjectPtr<UMaterialInterface> HighlightMaterial;
	
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
	
	UPROPERTY(VisibleInstanceOnly, Category = "Resource")
	FCPResourceNodeKey NodeKey;
	
	UPROPERTY(EditDefaultsOnly, Category = "Resource")
	TSubclassOf<ACPDroppedItemBase> DroppedItemClass;
	
	UPROPERTY(Transient)
	TObjectPtr<UCPResourceDefinition> ResourceDefinition;
	
	float InspectDuration = 1.f;
};
