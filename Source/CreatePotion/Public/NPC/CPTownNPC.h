#pragma once

#include "CoreMinimal.h"
#include "NPC/CPBaseNPC.h"
#include "CPTownNPC.generated.h"

UCLASS()
class CREATEPOTION_API ACPTownNPC : public ACPBaseNPC
{
	GENERATED_BODY()

public:
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() override;
};
