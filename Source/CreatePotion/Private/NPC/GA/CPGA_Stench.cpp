#include "NPC/GA/CPGA_Stench.h"
#include "NPC/CPBaseNPC.h"
#include "NPC/Components/CPNPCVisualComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Data/NPC/CPNPCDataAsset.h"

UCPGA_Stench::UCPGA_Stench()
{
	EffectTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Stench"));
	DurationWorldMinutes = 60.0f;
	Magnitude = 1.5f; 
}

void UCPGA_Stench::ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude)
{
	if (!NPC) return;

	if (!bActive)
	{
		Cleanup();
		return;
	}

	if (!StenchNiagaraSystem || !NPC->GetMesh()) return;

	CachedTargetNPC = NPC;
	if (InMagnitude > 0.01f)
	{
		Magnitude = InMagnitude;
	}

	if (!IsValid(SpawnedNiagaraComp))
	{
		SpawnedNiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			StenchNiagaraSystem, NPC->GetMesh(), AttachSocketName,
			FVector::ZeroVector, FRotator::ZeroRotator, FVector::OneVector,
			EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None, true
		);
		SpawnedNiagaraComp->SetUsingAbsoluteScale(true);

		if (UAbilitySystemComponent* ASC = NPC->GetAbilitySystemComponent())
		{
			const FGameplayTag GiantTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Giant"));
			GiantTagDelegateHandle = ASC->RegisterGameplayTagEvent(GiantTag, EGameplayTagEventType::NewOrRemoved)
				.AddLambda([this](const FGameplayTag, int32)
					{
						if (UWorld* World = GetWorld())
						{
							World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UCPGA_Stench::UpdateScale));
						}
					});
		}
	}

	UpdateScale();
}

void UCPGA_Stench::UpdateScale()
{
	ACPBaseNPC* NPC = CachedTargetNPC.Get();
	if (!NPC || !IsValid(SpawnedNiagaraComp)) return;

	const float GiantScale = NPC->GetVisualComponent() ? NPC->GetVisualComponent()->GetCurrentScaleMultiplier() : 1.0f;

	float NPCDataScale = 1.0f;
	if (const UCPNPCDataAsset* DataAsset = NPC->GetNPCData())
	{
		NPCDataScale = (DataAsset->VisualScaleMultiplier > 0.01f) ? DataAsset->VisualScaleMultiplier : 1.0f;
	}

	const float FinalScale = Magnitude * (GiantScale > 0.0f ? GiantScale : 1.0f) * NPCDataScale;

	SpawnedNiagaraComp->SetVariableFloat(FName("Scale"), FinalScale);
}

void UCPGA_Stench::Cleanup()
{
	if (ACPBaseNPC* NPC = CachedTargetNPC.Get())
	{
		if (UAbilitySystemComponent* ASC = NPC->GetAbilitySystemComponent())
		{
			if (GiantTagDelegateHandle.IsValid())
			{
				const FGameplayTag GiantTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Giant"));
				ASC->RegisterGameplayTagEvent(GiantTag, EGameplayTagEventType::NewOrRemoved).Remove(GiantTagDelegateHandle);
				GiantTagDelegateHandle.Reset();
			}
		}
	}

	if (IsValid(SpawnedNiagaraComp))
	{
		SpawnedNiagaraComp->Deactivate();
		SpawnedNiagaraComp = nullptr;
	}

	CachedTargetNPC = nullptr;
}

void UCPGA_Stench::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Cleanup();
	Super::OnRemoveAbility(ActorInfo, Spec);
}