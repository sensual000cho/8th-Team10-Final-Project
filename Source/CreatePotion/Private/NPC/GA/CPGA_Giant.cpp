#include "NPC/GA/CPGA_Giant.h"
#include "NPC/CPBaseNPC.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "Components/SkeletalMeshComponent.h"

UCPGA_Giant::UCPGA_Giant()
{
	EffectTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Giant"));
	ImmunityTag = FGameplayTag::RequestGameplayTag(FName("Immunity.Potion.Giant"));
	DurationWorldMinutes = 60;
	Magnitude = 3.0f;
}

void UCPGA_Giant::ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude)
{
	if (!NPC || !NPC->GetMesh()) return;

	UE_LOG(LogTemp, Warning, TEXT("[GA Execution] 1. Giant ApplyVisual START (Frame: %llu, Current Mesh Scale: %s)"),
		GFrameCounter,
		*NPC->GetMesh()->GetRelativeScale3D().ToString());

	const FVector BaseScale = NPC->GetBaseMeshScale();
	const FVector TargetScale = bActive ? (BaseScale * InMagnitude) : BaseScale;

	NPC->GetMesh()->SetRelativeScale3D(TargetScale);
	NPC->FitCapsuleToMesh(NPC->GetMesh()->GetSkeletalMeshAsset());

	UE_LOG(LogTemp, Warning, TEXT("[GA Execution] 1. Giant ApplyVisual END (New Mesh Scale: %s)"),
		*NPC->GetMesh()->GetRelativeScale3D().ToString());
}