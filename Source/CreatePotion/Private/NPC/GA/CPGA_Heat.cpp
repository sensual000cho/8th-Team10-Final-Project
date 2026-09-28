#include "NPC/GA/CPGA_Heat.h"
#include "NPC/CPBaseNPC.h"
#include "NPC/Components/CPNPCVisualComponent.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"

UCPGA_Heat::UCPGA_Heat()
{
	EffectTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Heat"));
	DurationWorldMinutes = 60;
	Magnitude = 1.0f;
}

void UCPGA_Heat::ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude)
{
	Super::ApplyVisual(NPC, bActive, InMagnitude);

	if (!NPC) { return; }

	if (!bActive)
	{
		Cleanup();
		return;
	}

	USkeletalMeshComponent* ParentMesh = NPC->GetMesh();
	if (!HeatMesh || !ParentMesh) { return; }

	CachedTargetNPC = NPC;
	if (InMagnitude > 0.01f)
	{
		Magnitude = InMagnitude;
	}

	const UCPNPCDataAsset* Data = NPC->GetNPCData();

	FName TargetSocketName = AttachSocketName;
	if (AttachSocketName != NAME_None && !ParentMesh->DoesSocketExist(AttachSocketName))
	{
		TargetSocketName = NAME_None;
	}

	if (!IsValid(HeatMeshComp))
	{
		FName UniqueCompName = MakeUniqueObjectName(NPC, UStaticMeshComponent::StaticClass(), TEXT("HeatVisualMeshComp"));
		HeatMeshComp = NewObject<UStaticMeshComponent>(NPC, UniqueCompName);
		if (!HeatMeshComp) { return; }

		HeatMeshComp->SetStaticMesh(HeatMesh);
		HeatMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HeatMeshComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		HeatMeshComp->SetGenerateOverlapEvents(false);
		HeatMeshComp->SetCanEverAffectNavigation(false);

		HeatMeshComp->AttachToComponent(ParentMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetSocketName);
		HeatMeshComp->SetAbsolute(false, false, true);

		if (UAbilitySystemComponent* ASC = NPC->GetAbilitySystemComponent())
		{
			const FGameplayTag GiantTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Giant"));
			GiantTagDelegateHandle = ASC->RegisterGameplayTagEvent(GiantTag, EGameplayTagEventType::NewOrRemoved)
				.AddLambda([this](const FGameplayTag, int32)
					{
						if (UWorld* World = GetWorld())
						{
							World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UCPGA_Heat::UpdateScale));
						}
					});
		}
	}

	if (!HeatMeshComp->IsRegistered())
	{
		HeatMeshComp->RegisterComponent();
	}

	HeatMeshComp->SetRelativeLocation(RelativeTransform.GetLocation());
	HeatMeshComp->SetRelativeRotation(RelativeTransform.GetRotation());

	UpdateScale();

	FLinearColor C1 = FLinearColor::Red;
	FLinearColor C2 = FLinearColor::Yellow;
	FLinearColor C3 = FLinearColor::White;

	if (Data)
	{
		C1 = Data->HeatColorData.Color1;
		C2 = Data->HeatColorData.Color2;
		C3 = Data->HeatColorData.Color3;
	}

	UMaterialInterface* BaseMat = HeatMaterial ? HeatMaterial.Get() : HeatMeshComp->GetMaterial(0);
	if (BaseMat)
	{
		UMaterialInstanceDynamic* HeatMID = UMaterialInstanceDynamic::Create(BaseMat, HeatMeshComp);
		if (HeatMID)
		{
			HeatMID->SetVectorParameterValue(FName("Color1"), C1);
			HeatMID->SetVectorParameterValue(FName("Color2"), C2);
			HeatMID->SetVectorParameterValue(FName("Color3"), C3);
			HeatMeshComp->SetMaterial(0, HeatMID);
		}
	}
}

void UCPGA_Heat::UpdateScale()
{
	ACPBaseNPC* NPC = CachedTargetNPC.Get();
	if (!NPC || !IsValid(HeatMeshComp)) { return; }

	const float GiantScale = NPC->GetVisualComponent() ? NPC->GetVisualComponent()->GetCurrentScaleMultiplier() : 1.0f;

	float NPCDataScale = 1.0f;
	if (const UCPNPCDataAsset* DataAsset = NPC->GetNPCData())
	{
		NPCDataScale = (DataAsset->VisualScaleMultiplier > 0.01f) ? DataAsset->VisualScaleMultiplier : 1.0f;
	}

	const FVector FinalScale = RelativeTransform.GetScale3D() * Magnitude * (GiantScale > 0.0f ? GiantScale : 1.0f) * NPCDataScale;

	HeatMeshComp->SetWorldScale3D(FinalScale);
}

void UCPGA_Heat::Cleanup()
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

	if (IsValid(HeatMeshComp))
	{
		HeatMeshComp->DestroyComponent();
		HeatMeshComp = nullptr;
	}

	CachedTargetNPC = nullptr;
}

void UCPGA_Heat::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Cleanup();
	Super::OnRemoveAbility(ActorInfo, Spec);
}