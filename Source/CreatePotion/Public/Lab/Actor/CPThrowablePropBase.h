#pragma once

#include "CoreMinimal.h"
#include "GameCore/Interface/CPInteractable.h"
#include "GameFramework/Actor.h"
#include "CPThrowablePropBase.generated.h"

class UStaticMeshComponent;

UCLASS(Abstract)
class CREATEPOTION_API ACPThrowablePropBase : public AActor, public ICPInteractable
{
	GENERATED_BODY()

public:
	ACPThrowablePropBase();
	
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual FName GetInteractionName_Implementation() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prop")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
};