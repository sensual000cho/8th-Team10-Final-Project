#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Resource/CPResourceType.h"
#include "CPResourceNodeActor.generated.h"

class UCPResourceDefinition;

UCLASS()
class CREATEPOTION_API ACPResourceNodeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ACPResourceNodeActor();

	void InitializeResource(const FCPResourceNodeKey& InNodeKey, UCPResourceDefinition* InDefinition);
	
private:
	void ApplyDefinition();
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Resource")
	FCPResourceNodeKey NodeKey;
	
	UPROPERTY(Transient)
	TObjectPtr<UCPResourceDefinition> ResourceDefinition;
};
