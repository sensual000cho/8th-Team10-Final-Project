#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_PotionReactionBase.h"
#include "CPGA_FartLaunch.generated.h"

UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_FartLaunch : public UCPGA_PotionReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_FartLaunch();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	float FloatDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	float LaunchForce;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|FartLaunch")
	FGameplayTag FartLaunchCueTag;

private:
	UFUNCTION()
	void OnLaunchFinished();
};