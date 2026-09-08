#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_PersistentReactionBase.h"
#include "CPGA_Slime.generated.h"

class USkeletalMesh;
class UAnimSequence;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class CREATEPOTION_API UCPGA_Slime : public UCPGA_PersistentReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_Slime();

	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:
	virtual void ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	TObjectPtr<USkeletalMesh> SlimeMesh;

	// 슬라임 단독 머티리얼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	TObjectPtr<UMaterialInterface> SlimeBaseMaterial;

	// 슬라임 + 바운시 결합 머티리얼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	TObjectPtr<UMaterialInterface> SlimeBouncyMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	FName InnerColorParamName = FName("SlimeInnerColor");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	FName OuterColorParamName = FName("SlimeOuterColor");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	float CapsuleRadius = 34.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	float CapsuleHalfHeight = 88.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	float MeshZOffset = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	FGameplayTag BouncyTag;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CachedSlimeBaseMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CachedSlimeBouncyMID;

	void SetupCompositeRule(ACPBaseNPC* NPC);
};