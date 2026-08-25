// GA_CPAimThrowProp.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_CPAimThrowProp.generated.h"

class ACPThrowablePropBase;
class UCPCarryComponent;
class UCPTrajectoryPreviewComponent;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class CREATEPOTION_API UGA_CPAimThrowProp : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_CPAimThrowProp();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	static const FGameplayTag& GetAimStartEventTag();
	static const FGameplayTag& GetAimEndEventTag();
	static const FGameplayTag& GetAimingStateTag();
	static const FGameplayTag& GetAimAbilityTag();

private:
	UFUNCTION()
	void HandleAimEndEvent(FGameplayEventData Payload);

	UFUNCTION()
	void HandleHeldPropChanged(ACPThrowablePropBase* NewHeldProp);

	void EndCurrentAim(bool bWasCancelled);

private:
	UPROPERTY()
	TObjectPtr<UCPCarryComponent> BoundCarryComponent;
	
	UPROPERTY()
	TObjectPtr<UCPTrajectoryPreviewComponent> TrajectoryPreviewComponent;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitAimEndTask;
};
