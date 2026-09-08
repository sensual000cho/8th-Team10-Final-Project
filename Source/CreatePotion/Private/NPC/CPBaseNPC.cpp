#include "NPC/CPBaseNPC.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "Data/NPC/CPGADataAsset.h"
#include "GameInstance/Subsystem/CPNPCSubsystem.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NPC/CPNPCVisualComponent.h"

ACPBaseNPC::ACPBaseNPC()
{
	PrimaryActorTick.bCanEverTick = false;
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
	if (GetMesh())
	{
		GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bRunPhysicsWithNoController = true;
	}

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	VisualComponent = CreateDefaultSubobject<UCPNPCVisualComponent>(TEXT("VisualComponent"));
}

UAbilitySystemComponent* ACPBaseNPC::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FName ACPBaseNPC::GetPotionNPCId() const
{
	if (NPCData && NPCData->NPCName != NAME_None)
	{
		return NPCData->NPCName;
	}
	return NAME_None;
}

UCPNPCSubsystem* ACPBaseNPC::GetNPCSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UCPNPCSubsystem>() : nullptr;
}

void ACPBaseNPC::BeginPlay()
{
	Super::BeginPlay();

	InitializeFromDataAsset();

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		GrantPotionReactionAbilities();
	}

	if (UCPNPCSubsystem* Subsystem = GetNPCSubsystem())
	{
		Subsystem->RegisterActiveNPC(GetPotionNPCId(), this);
	}

	CatchUpPersistentPotionEffects();
}

void ACPBaseNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UCPNPCSubsystem* Subsystem = GetNPCSubsystem())
	{
		Subsystem->UnregisterActiveNPC(GetPotionNPCId(), this);
	}

	VisualDelegateMap.Empty();
	Super::EndPlay(EndPlayReason);
}

void ACPBaseNPC::InitializeFromDataAsset()
{
	if (!NPCData)
	{
		return;
	}

	USkeletalMesh* LoadedMesh = NPCData->NPCMesh.LoadSynchronous();
	UAnimSequence* LoadedAnim = NPCData->IdleAnimation.LoadSynchronous();

	if (GetMesh())
	{
		GetMesh()->SetRelativeRotation(NPCData->MeshRotationOffset);
	}

	if (VisualComponent)
	{
		VisualComponent->InitializeVisual(GetMesh(), GetCapsuleComponent(), LoadedMesh, LoadedAnim, NPCData->MeshScale, NPCData->CapsuleRadiusRatio);
	}

	if (NPCData->NPCName != NAME_None)
	{
		Tags.AddUnique(NPCData->NPCName);
	}
}

void ACPBaseNPC::RegisterVisualDelegate(FGameplayTag EffectTag, const FOnPotionVisualUpdateDelegate& Delegate)
{
	if (EffectTag.IsValid())
	{
		VisualDelegateMap.Add(EffectTag, Delegate);
	}
}

void ACPBaseNPC::UnregisterVisualDelegate(FGameplayTag EffectTag)
{
	if (EffectTag.IsValid())
	{
		VisualDelegateMap.Remove(EffectTag);
	}
}

void ACPBaseNPC::RegisterPersistentPotionEffect(FGameplayTag EffectTag, int32 InDurationWorldMinutes, float Magnitude)
{
	UCPNPCSubsystem* Subsystem = GetNPCSubsystem();
	const FName NPCId = GetPotionNPCId();

	if (!Subsystem || NPCId.IsNone() || !EffectTag.IsValid())
	{
		return;
	}

	Subsystem->RegisterNPCEffect(NPCId, EffectTag, InDurationWorldMinutes, Magnitude);
}

void ACPBaseNPC::HandlePotionEffectApplied(FGameplayTag EffectTag, int64 ExpiresAtWorldMinute, float Magnitude)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(EffectTag);
	}

	ApplyPotionEffectVisual(EffectTag, true, Magnitude);
}

void ACPBaseNPC::HandlePotionEffectExpired(FGameplayTag EffectTag)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(EffectTag);
	}

	ApplyPotionEffectVisual(EffectTag, false, 1.0f);
}

void ACPBaseNPC::CatchUpPersistentPotionEffects()
{
	UCPNPCSubsystem* Subsystem = GetNPCSubsystem();
	const FName NPCId = GetPotionNPCId();
	if (!Subsystem || NPCId.IsNone()) return;

	FCPNPCEffectSaveData SaveData;
	if (Subsystem->GetNPCEffectData(NPCId, SaveData))
	{
		for (const auto& EffectPair : SaveData.ActiveEffects)
		{
			if (AbilitySystemComponent)
			{
				AbilitySystemComponent->AddLooseGameplayTag(EffectPair.Key);
			}
			ApplyPotionEffectVisual(EffectPair.Key, true, EffectPair.Value.Magnitude);
		}
	}
}

void ACPBaseNPC::ApplyPotionEffectVisual_Implementation(FGameplayTag EffectTag, bool bActive, float Magnitude)
{
	if (const FOnPotionVisualUpdateDelegate* FoundDelegate = VisualDelegateMap.Find(EffectTag))
	{
		if (FoundDelegate->IsBound())
		{
			FoundDelegate->Execute(bActive, Magnitude);
		}
	}
}

void ACPBaseNPC::GrantPotionReactionAbilities()
{
	if (!IsValid(AbilitySystemComponent) || !HasAuthority())
	{
		return;
	}

	if (CommonAbilitySet)
	{
		for (const TSubclassOf<UGameplayAbility>& AbilityClass : CommonAbilitySet->GrantedAbilities)
		{
			if (AbilityClass && !AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass))
			{
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
			}
		}
	}
}

void ACPBaseNPC::OnInteract_Implementation(AActor* Interactor) {}
FText ACPBaseNPC::GetInteractionPrompt_Implementation() { return FText::GetEmpty(); }
bool ACPBaseNPC::CanInteract_Implementation(AActor* Interactor) { return false; }
FName ACPBaseNPC::GetInteractionName_Implementation()
{
	if (NPCData && NPCData->NPCName != NAME_None)
	{
		return NPCData->NPCName;
	}
	return GetFName();
}

