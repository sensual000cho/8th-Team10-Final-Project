#pragma once

#include "CoreMinimal.h"
#include "GameCore/Interface/CPInteractable.h"
#include "GameFramework/Actor.h"
#include "CPThrowablePropBase.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS(Abstract)
class CREATEPOTION_API ACPThrowablePropBase : public AActor, public ICPInteractable
{
	GENERATED_BODY()

public:
	ACPThrowablePropBase();
	
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual FName GetInteractionName_Implementation() override;

	bool AttachAsHeld(USceneComponent* CarryAnchor);
	void DetachAsHeld(const FVector& DropLocation);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prop")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
};
