#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_PersistentReactionBase.h"
#include "CPGA_Heat.generated.h"

class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class ACPBaseNPC;

UCLASS()
class CREATEPOTION_API UCPGA_Heat : public UCPGA_PersistentReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_Heat();

protected:
	virtual void ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

private:
	void UpdateScale();
	void Cleanup();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual|Heat")
	TObjectPtr<UStaticMesh> HeatMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual|Heat")
	TObjectPtr<UMaterialInterface> HeatMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual|Heat")
	FName AttachSocketName = FName("Heat_Socket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual|Heat")
	FTransform RelativeTransform = FTransform::Identity;

private:
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> HeatMeshComp;

	TWeakObjectPtr<ACPBaseNPC> CachedTargetNPC;

	FDelegateHandle GiantTagDelegateHandle;
};