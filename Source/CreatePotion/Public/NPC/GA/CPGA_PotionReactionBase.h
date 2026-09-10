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

	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:
	static bool ExtractImpactHitResult(const FGameplayEventData* TriggerEventData, FHitResult& OutHitResult);
	ACPBaseNPC* GetOwningPotionNPC(const FGameplayAbilityActorInfo* ActorInfo) const;

	// NPC 유효성 + 서버 권한 검증. 실패 시 어빌리티를 즉시 취소 종료하고 nullptr 반환.
	ACPBaseNPC* GetValidatedOwningPotionNPC(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion")
	FGameplayTag ImmunityTag;
	
	//디버그용
	void PrintPotionEventLog(const FGameplayEventData* TriggerEventData) const;

};