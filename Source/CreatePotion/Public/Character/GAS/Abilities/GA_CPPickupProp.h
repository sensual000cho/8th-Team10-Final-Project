// GA_CPPickupProp.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_CPPickupProp.generated.h"

class UCPCarryComponent;

/*
 * Event.Carry.Pickup 이벤트를 받아 Throwable Prop을 집는 Ability.
 * Held 참조와 실제 부착은 CPCarryComponent가 담당하고,
 * 이 Ability는 검증과 상태 전환만 담당.
 */
UCLASS(Blueprintable)
class CREATEPOTION_API UGA_CPPickupProp : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_CPPickupProp();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UCPCarryComponent* FindCarryComponent(const FGameplayAbilityActorInfo* ActorInfo) const;
	static const FGameplayTag& GetPickupEventTag();
};
