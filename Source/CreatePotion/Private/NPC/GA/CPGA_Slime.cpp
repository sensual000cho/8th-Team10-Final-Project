#include "NPC/GA/CPGA_Slime.h"
#include "NPC/CPBaseNPC.h"
#include "NPC/Components/CPNPCVisualComponent.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "Materials/MaterialInstanceDynamic.h"

UCPGA_Slime::UCPGA_Slime()
{
	EffectTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Slime"));
	ImmunityTag = FGameplayTag::RequestGameplayTag(FName("Immunity.Potion.Slime"));
	BouncyTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Bouncy"));
	DurationWorldMinutes = 60;
	Magnitude = 1.0f;
}

void UCPGA_Slime::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (ACPBaseNPC* NPC = Cast<ACPBaseNPC>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr))
	{
		SetupCompositeRule(NPC);
	}
}

void UCPGA_Slime::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (ACPBaseNPC* NPC = Cast<ACPBaseNPC>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr))
	{
		if (UCPNPCVisualComponent* VisualComp = NPC->GetVisualComponent())
		{
			FGameplayTagContainer Tags;
			Tags.AddTag(EffectTag);
			Tags.AddTag(BouncyTag);
			VisualComp->UnregisterCompositeRule(Tags);
		}
	}

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UCPGA_Slime::SetupCompositeRule(ACPBaseNPC* NPC)
{
	if (!NPC || !SlimeBouncyMaterial) { return; }
	UCPNPCVisualComponent* VisualComp = NPC->GetVisualComponent();
	if (!VisualComp) { return; }

	if (!IsValid(CachedSlimeBouncyMID))
	{
		CachedSlimeBouncyMID = UMaterialInstanceDynamic::Create(SlimeBouncyMaterial, this);
	}

	if (CachedSlimeBouncyMID)
	{
		if (const UCPNPCDataAsset* Data = NPC->GetNPCData())
		{
			CachedSlimeBouncyMID->SetVectorParameterValue(InnerColorParamName, Data->SlimeInnerColor);
			CachedSlimeBouncyMID->SetVectorParameterValue(OuterColorParamName, Data->SlimeOuterColor);
		}

		FCompositeMaterialRule Rule;
		Rule.RequiredTags.AddTag(EffectTag); // State.Effect.Slime
		Rule.RequiredTags.AddTag(BouncyTag); // State.Effect.Bouncy
		Rule.ResultMaterial = CachedSlimeBouncyMID;
		Rule.Priority = 10;

		VisualComp->RegisterCompositeRule(Rule);
	}
}

void UCPGA_Slime::ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude)
{
	if (!NPC) { return; }
	UCPNPCVisualComponent* VisualComp = NPC->GetVisualComponent();
	if (!VisualComp) { return; }

	if (bActive)
	{
		const UCPNPCDataAsset* Data = NPC->GetNPCData();
		const float ScaleMultiplier = (Data && Data->SlimeScaleMultiplier > 0.0f) ? Data->SlimeScaleMultiplier : 1.0f;

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

				VisualComp->PushMaterialOverride(0, EffectTag, CachedSlimeBaseMID);
			}
		}

		VisualComp->PushMeshOverride(
			EffectTag,
			SlimeMesh,
			nullptr,
			0.3f,
			MeshZOffset * ScaleMultiplier,
			FVector(ScaleMultiplier),
			CapsuleRadius * ScaleMultiplier,
			CapsuleHalfHeight * ScaleMultiplier,
			true
		);
	}
	else
	{
		VisualComp->PopMeshOverride(EffectTag);
		VisualComp->PopMaterialOverride(0, EffectTag);
	}
}