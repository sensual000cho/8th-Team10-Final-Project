#pragma once

#include "CoreMinimal.h"
#include "Components/CPInteractableComponent.h"
#include "CPNPCComponent.generated.h"

class ACPQuestNPC;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPNPCComponent : public UCPInteractableComponent
{
	GENERATED_BODY()

public:
	UCPNPCComponent();
	virtual void BeginPlay() override;
	
	virtual void BeginFocus(AActor* Interactor) override;
	virtual void EndFocus(AActor* Interactor) override;
	
	virtual bool TryInteract(AActor* Interactor) override;

};
