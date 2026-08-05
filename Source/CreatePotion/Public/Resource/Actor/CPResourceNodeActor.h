#pragma once

#include "CoreMinimal.h"
#include "Data/CPForageableItemData.h"
#include "GameCore/Interface/CPInteractable.h"
#include "GameFramework/Actor.h"
#include "Resource/CPResourceType.h"
#include "CPResourceNodeActor.generated.h"

class UCPResourceDefinition;

UCLASS()
class CREATEPOTION_API ACPResourceNodeActor : public AActor, public ICPInteractable
{
	GENERATED_BODY()
	
public:	
	ACPResourceNodeActor();

	// 초기화
	void InitializeResource(const FCPResourceNodeKey& InNodeKey, UCPResourceDefinition* InDefinition);
	
	// 인터랙션 인터페이스 관련
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	
	virtual FText GetInteractionPrompt_Implementation() override;
	
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	
private:
	// 채집물에 해당하는 정보 적용
	void ApplyDefinition();
	
	// 채집
	void Harvest(AActor* Interactor);
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Resource")
	FCPResourceNodeKey NodeKey;
	
	UPROPERTY(Transient)
	TObjectPtr<UCPResourceDefinition> ResourceDefinition;
};
