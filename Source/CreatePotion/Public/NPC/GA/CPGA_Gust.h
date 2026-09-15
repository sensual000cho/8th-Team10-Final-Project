#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_PersistentReactionBase.h"
#include "CPGA_Gust.generated.h"

UCLASS()
class CREATEPOTION_API UCPGA_Gust : public UCPGA_PersistentReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_Gust();

protected:
	virtual void ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude) override;
};