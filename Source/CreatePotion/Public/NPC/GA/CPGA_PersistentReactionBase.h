#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_PotionReactionBase.h"
#include "CPGA_PersistentReactionBase.generated.h"

class ACPBaseNPC;

UCLASS(Abstract, Blueprintable)
class CREATEPOTION_API UCPGA_PersistentReactionBase : public UCPGA_PotionReactionBase
{
	GENERATED_BODY()

public:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 이 효과가 NPC에게 어떻게 보일지 서브클래스가 구현, bActive=false면 원상복구.
	virtual void ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude) {}

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|PersistentEffect")
	FGameplayTag EffectTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|PersistentEffect")
	int32 DurationWorldMinutes = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|PersistentEffect")
	float Magnitude = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion|PersistentEffect")
	FGameplayTag ImmunityTag;

private:
	// ApplyVisual에 NPC를 매번 다시 찾아 넘기지 않도록 OnGiveAbility 시점에 캐싱.
	TWeakObjectPtr<ACPBaseNPC> CachedNPC;

	void HandleApplyVisual(bool bActive, float InMagnitude);
};