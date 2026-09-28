#include "NPC/GA/CPGA_Slime.h"
#include "NPC/CPBaseNPC.h"
#include "NPC/Components/CPNPCVisualComponent.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"

UCPGA_Slime::UCPGA_Slime()
{
	EffectTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Slime"));
	ImmunityTag = FGameplayTag::RequestGameplayTag(FName("Immunity.Potion.Slime"));
	DurationWorldMinutes = 60;
	Magnitude = 1.0f;
}

void UCPGA_Slime::ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude)
{
	if (!NPC) { return; }
	UCPNPCVisualComponent* VisualComp = NPC->GetVisualComponent();
	if (!VisualComp) { return; }

	if (bActive)
	{
		const UCPNPCDataAsset* Data = NPC->GetNPCData();
		const float ScaleMultiplier = (Data && Data->VisualScaleMultiplier > 0.0f) ? Data->VisualScaleMultiplier : 1.0f;

		if (SlimeBaseMaterial)
		{
			if (!IsValid(CachedSlimeBaseMID))
			{
				CachedSlimeBaseMID = UMaterialInstanceDynamic::Create(SlimeBaseMaterial, this);
			}

			if (CachedSlimeBaseMID)
			{
				if (Data)
				{
					CachedSlimeBaseMID->SetVectorParameterValue(InnerColorParamName, Data->SlimeInnerColor);
					CachedSlimeBaseMID->SetVectorParameterValue(OuterColorParamName, Data->SlimeOuterColor);
				}

				VisualComp->PushMaterialOverride(0, EffectTag, CachedSlimeBaseMID, 1);
			}
		}

		VisualComp->PushMeshOverride(
			EffectTag,
			SlimeMesh,
			SlimeAnimClass,
			0.3f,
			FVector(ScaleMultiplier),
			CapsuleRadius * ScaleMultiplier,
			CapsuleHalfHeight * ScaleMultiplier,
			SlimeRotationOffset
		);

		if (Data && !Data->SlimeAccessory.AccessoryMesh.IsNull())
		{
			UStaticMesh* AccMesh = Data->SlimeAccessory.AccessoryMesh.LoadSynchronous();
			if (AccMesh)
			{
				VisualComp->AttachSlimeAccessory(
					AccMesh,
					Data->SlimeAccessory.SocketName,
					Data->SlimeAccessory.RelativeTransform
				);
			}
		}
	}
	else
	{
		VisualComp->DetachSlimeAccessory();
		VisualComp->PopMeshOverride(EffectTag);
		VisualComp->PopMaterialOverride(0, EffectTag);
	}
}