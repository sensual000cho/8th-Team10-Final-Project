#pragma once

#include "CoreMinimal.h"
#include "NPC/GA/CPGA_PersistentReactionBase.h"
#include "CPGA_Bouncy.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class CREATEPOTION_API UCPGA_Bouncy : public UCPGA_PersistentReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_Bouncy();

protected:
	virtual void ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Bouncy")
	TObjectPtr<UMaterialInterface> BouncyOverlayMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Bouncy")
	FName InnerColorParamName = FName("SlimeInnerColor");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Bouncy")
	FName OuterColorParamName = FName("SlimeOuterColor");

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CachedBouncyMID;
};