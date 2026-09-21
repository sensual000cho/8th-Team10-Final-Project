#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_PersistentReactionBase.h"
#include "CPGA_Stench.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class ACPBaseNPC;

UCLASS()
class CREATEPOTION_API UCPGA_Stench : public UCPGA_PersistentReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_Stench();

protected:
	virtual void ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

private:
	void UpdateScale();
	void Cleanup();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Visual|Stench")
	TObjectPtr<UNiagaraSystem> StenchNiagaraSystem;

	UPROPERTY(EditDefaultsOnly, Category = "Visual|Stench")
	FName AttachSocketName = FName("Stench_Socket");

private:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> SpawnedNiagaraComp;

	TWeakObjectPtr<ACPBaseNPC> CachedTargetNPC;
	FDelegateHandle GiantTagDelegateHandle;
};