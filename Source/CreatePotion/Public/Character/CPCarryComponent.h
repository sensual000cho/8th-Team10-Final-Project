// CPCarryComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CPCarryComponent.generated.h"


UCLASS( ClassGroup=(Carry), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPCarryComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UCPCarryComponent();

	// Prop을 이 컴포넌트 위치에 부착
	UFUNCTION(BlueprintCallable, Category = "Carry")
	bool AttachProp(ACPThrowablePropBase* Prop);

	// Prop을 이 컴포넌트에서 분리하고 월드에 배치
	UFUNCTION(BlueprintCallable, Category = "Carry")
	bool DetachProp(ACPThrowablePropBase* Prop, const FVector& DropLocation);	
};
