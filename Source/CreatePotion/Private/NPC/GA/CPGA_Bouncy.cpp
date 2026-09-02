#include "NPC/GA/CPGA_Bouncy.h"
#include "NPC/CPBaseNPC.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UCPGA_Bouncy::UCPGA_Bouncy()
{
	EffectTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Bouncy"));
	ImmunityTag = FGameplayTag::RequestGameplayTag(FName("Immunity.Potion.Bouncy"));
	DurationWorldMinutes = 60;
	Magnitude = 1.0f;
}

void UCPGA_Bouncy::ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude)
{
	if (!NPC || !NPC->GetMesh())
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = NPC->GetMesh();

	if (bActive)
	{
		if (!bHasCachedOriginalMaterial)
		{
			CachedOriginalMaterial = MeshComp->GetMaterial(0);
			bHasCachedOriginalMaterial = true;
		}

		if (!BouncyOverlayMaterial)
		{
			UE_LOG(LogTemp, Error, TEXT("[UCPGA_Bouncy] BouncyOverlayMaterial is NULL!"));
			return;
		}

		if (!IsValid(CachedBouncyMID))
		{
			CachedBouncyMID = UMaterialInstanceDynamic::Create(BouncyOverlayMaterial, this);
		}

		if (CachedBouncyMID)
		{
			if (const UCPNPCDataAsset* Data = NPC->GetNPCData())
			{
				CachedBouncyMID->SetVectorParameterValue(InnerColorParamName, Data->SlimeInnerColor);
				CachedBouncyMID->SetVectorParameterValue(OuterColorParamName, Data->SlimeOuterColor);
			}
			MeshComp->SetMaterial(0, CachedBouncyMID);
		}
	}
	else
	{
		MeshComp->SetMaterial(0, bHasCachedOriginalMaterial ? CachedOriginalMaterial : nullptr);
	}
}