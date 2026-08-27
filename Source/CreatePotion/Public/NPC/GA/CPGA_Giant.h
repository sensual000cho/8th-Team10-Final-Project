#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_PotionReactionBase.h"
#include "CPGA_Giant.generated.h"


UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_Giant : public UCPGA_PotionReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_Giant();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// NPCSubsystem에 등록할 월드시간 기준 지속시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant")
	int32 DurationWorldMinutes;

	// 실제 확대 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant")
	float GiantScaleMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant")
	FGameplayTag GiantCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|Giant")
	FGameplayTag GiantStateTag;
};