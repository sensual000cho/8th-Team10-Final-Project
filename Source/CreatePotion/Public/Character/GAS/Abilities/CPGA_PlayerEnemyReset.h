#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CPGA_PlayerEnemyReset.generated.h"

/** Player와 Enemy의 태그 목록에 해당하는 지속 효과만 중화한다. */
UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_PlayerEnemyReset : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCPGA_PlayerEnemyReset();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// GA의 Asset Tags, GE의 Asset/Granted Tags를 비교하며 하위 태그도 포함한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Reset")
	FGameplayTagContainer ResettableEffectTags;

	// 중화 대상과 겹치면 제외 목록을 우선한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Reset")
	FGameplayTagContainer ExcludedEffectTags;

private:
	bool IsResettable(const FGameplayTagContainer& EffectTags) const;
};
