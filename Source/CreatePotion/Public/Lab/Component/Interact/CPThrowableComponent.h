#pragma once

#include "CoreMinimal.h"
#include "Components/CPInteractableComponent.h"
#include "CPThrowableComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPThrowableComponent : public UCPInteractableComponent
{
	GENERATED_BODY()

public:
	UCPThrowableComponent();

	virtual bool CanInteract() const override;
	virtual bool TryInteract(AActor* Interactor) override;

	void SetTargetName(FName InTargetName);
};
