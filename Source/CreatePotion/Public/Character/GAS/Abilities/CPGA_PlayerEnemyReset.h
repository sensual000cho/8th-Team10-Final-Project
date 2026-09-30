#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CPGA_PlayerEnemyReset.generated.h"

class USoundBase;

/** Player와 Enemy의 태그 목록에 해당하는 지속 효과만 중화한다. */
UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_PlayerEnemyReset : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCPGA_PlayerEnemyReset();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 효과 적용 성공 시 재생할 단발음. 미지정 시 생략
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> StartSFX;

	// 효과 종료 시 재생할 단발음. 미지정 시 생략
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> EndSFX;

	// GA의 Asset Tags, GE의 Asset/Granted Tags를 비교하며 하위 태그도 포함한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Reset")
	FGameplayTagContainer ResettableEffectTags;

	// 중화 대상과 겹치면 제외 목록을 우선한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Reset")
	FGameplayTagContainer ExcludedEffectTags;

private:
	bool IsResettable(const FGameplayTagContainer& EffectTags) const;
};
