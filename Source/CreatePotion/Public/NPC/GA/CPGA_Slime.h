#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_PersistentReactionBase.h"
#include "CPGA_Slime.generated.h"

class USkeletalMesh;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class CREATEPOTION_API UCPGA_Slime : public UCPGA_PersistentReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_Slime();

protected:
	virtual void ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	TObjectPtr<USkeletalMesh> SlimeMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	TSubclassOf<UAnimInstance> SlimeAnimClass;

	// 슬라임 단독 머티리얼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	TObjectPtr<UMaterialInterface> SlimeBaseMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	FName InnerColorParamName = FName("SlimeInnerColor");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	FName OuterColorParamName = FName("SlimeOuterColor");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	float CapsuleRadius = 34.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	float CapsuleHalfHeight = 88.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Slime")
	FRotator SlimeRotationOffset = FRotator(0.0f, 90.0f, 0.0f);

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CachedSlimeBaseMID;
};