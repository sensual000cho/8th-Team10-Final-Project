// GA_CPThrowProp.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_CPThrowProp.generated.h"

class UCPCarryComponent;

/*
 * 플레이어가 현재 들고 Prop을 투척하는 Ability.
 * Held 참조와 물리 투척은 CPCarryComponent가 담당하고,
 * 이 Ability는 활성화 조건, 투척 방향과 세기를 결정.
 */
UCLASS(Blueprintable)
class CREATEPOTION_API UGA_CPThrowProp : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_CPThrowProp();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//GAS 외부의 TrajectoryPreview 등과 소통을 편하게 하기 위한 매개변수 수정
	FVector CalculateThrowDirection(const AActor* AvatarActor, const APlayerController* PlayerController) const;
	//Trajectory 계산 위한 Get 함수
	float GetThrowSpeed() const;
	
private:
	UCPCarryComponent* FindCarryComponent(const FGameplayAbilityActorInfo* ActorInfo) const;
	static const FGameplayTag& GetThrowEventTag();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Carry|Throw", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "cm/s"))
	float ThrowSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Carry|Throw", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float UpwardBias;
};
