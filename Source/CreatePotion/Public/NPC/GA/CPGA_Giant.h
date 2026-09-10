#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_PersistentReactionBase.h"
#include "CPGA_Giant.generated.h"

UCLASS()
class CREATEPOTION_API UCPGA_Giant : public UCPGA_PersistentReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_Giant();

protected:
	virtual void ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude) override;
};