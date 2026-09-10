#include "NPC/GA/CPGA_Giant.h"
#include "NPC/CPBaseNPC.h"
#include "NPC/CPNPCVisualComponent.h"

UCPGA_Giant::UCPGA_Giant()
{
	EffectTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Giant"));
	ImmunityTag = FGameplayTag::RequestGameplayTag(FName("Immunity.Potion.Giant"));
	DurationWorldMinutes = 60;
	Magnitude = 3.0f;
}

void UCPGA_Giant::ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude)
{
	if (!NPC || !NPC->GetMesh()) { return; }

	if (UCPNPCVisualComponent* VisualComp = NPC->GetVisualComponent())
	{
		VisualComp->SetScaleMultiplier(bActive ? InMagnitude : 1.0f);
	}
}
