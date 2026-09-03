#pragma once

#include "CoreMinimal.h"
#include "NPC/CPBaseNPC.h"
#include "CPAnimalNPC.generated.h"

UCLASS()
class CREATEPOTION_API ACPAnimalNPC : public ACPBaseNPC
{
	GENERATED_BODY()

public:
	ACPAnimalNPC();

	virtual FName GetPotionNPCId() const override;

protected:
	UPROPERTY(Transient)
	mutable FName CachedPotionNPCId = NAME_None;
};