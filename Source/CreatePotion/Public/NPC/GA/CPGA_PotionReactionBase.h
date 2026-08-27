#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CPGA_PotionReactionBase.generated.h"

class ACPBaseNPC;

UCLASS(Abstract)
class CREATEPOTION_API UCPGA_PotionReactionBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCPGA_PotionReactionBase();

protected:
	static bool ExtractImpactHitResult(const FGameplayEventData* TriggerEventData, FHitResult& OutHitResult);
	ACPBaseNPC* GetOwningPotionNPC(const FGameplayAbilityActorInfo* ActorInfo) const;
	
	//디버그용
	void PrintPotionEventLog(const FGameplayEventData* TriggerEventData) const;

};