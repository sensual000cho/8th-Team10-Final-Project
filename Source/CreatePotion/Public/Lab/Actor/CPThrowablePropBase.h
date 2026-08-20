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
	
	// 머리 위에서 분리하고 물리 투척
	bool Throw(const FVector& Direction, float Speed);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prop")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	// Prop마다 머리 위 위치와 회전을 보정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop|Carry")
	FTransform HeldRelativeTransform;

	// 물리 활성화 전 투척 방향으로 조금 이동
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop|Throw")
	float ThrowStartOffset = 20.f;
};
