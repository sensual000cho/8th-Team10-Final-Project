#include "NPC/GA/CPGA_Bouncy.h"
#include "NPC/CPBaseNPC.h"
#include "NPC/Components/CPNPCVisualComponent.h"
#include "Data/NPC/CPNPCDataAsset.h"
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
	if (!NPC) { return; }
	UCPNPCVisualComponent* VisualComp = NPC->GetVisualComponent();
	if (!VisualComp) { return; }

	if (bActive)
	{
		if (!BouncyOverlayMaterial)
		{
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

			VisualComp->PushMaterialOverride(0, EffectTag, CachedBouncyMID);
		}
	}
	else
	{
		VisualComp->PopMaterialOverride(0, EffectTag);
	}
}