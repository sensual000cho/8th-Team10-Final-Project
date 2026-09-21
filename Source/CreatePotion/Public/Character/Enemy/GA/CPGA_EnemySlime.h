#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CPGA_EnemySlime.generated.h"

class ACPEnemyCharacter;

/** 같은 Enemy를 한 마리 생성하고 양쪽을 80% 크기로 줄이는 포션 반응. */
UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_EnemySlime : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCPGA_EnemySlime();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 원본과 복제본이 서로 겹치지 않도록 더할 여유 거리다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime", meta = (ClampMin = "1.0", Units = "cm"))
	float SplitSpacing = 20.0f;

	// 좌우로 튀어나올 때의 수평 속도와 수직 속도다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime", meta = (ClampMin = "0.0", Units = "cm/s"))
	float SplitHorizontalSpeed = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime", meta = (ClampMin = "0.0", Units = "cm/s"))
	float SplitUpwardSpeed = 180.0f;

private:
	// 원본 기준 1 → 0.8 → 0.64의 두 단계만 허용한다.
	static constexpr int32 MaxSplitCount = 2;
	static constexpr float SplitScaleMultiplier = 0.8f;

	// 좌우 두 위치가 모두 안전할 때만 분열을 진행한다.
	bool FindSplitLocations(const ACPEnemyCharacter& Enemy, FVector& OutOriginalLocation, FVector& OutCloneLocation) const;
	bool FindGroundedLocation(const ACPEnemyCharacter& Enemy, const FVector& Candidate, FVector& OutLocation) const;
};
