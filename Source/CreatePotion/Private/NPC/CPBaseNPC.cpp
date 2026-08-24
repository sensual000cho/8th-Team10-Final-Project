#include "NPC/CPBaseNPC.h"
#include "Data/CPNPCDataAsset.h"
#include "Data/CPGADataAsset.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimSequence.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/QuestManager.h"
#include "GameInstance/Subsystem/CPNPCSubsystem.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/CharacterMovementComponent.h"

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

	PersistentPotionEffectTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Effect.Giant")));
}

void ACPBaseNPC::OnInteract_Implementation(AActor* Interactor)
{
}

FText ACPBaseNPC::GetInteractionPrompt_Implementation()
{
	if (!CanInteract_Implementation(nullptr))
	{
		return FText::GetEmpty();
	}

	return FText::FromString(TEXT("F : 대화하기"));
}

bool ACPBaseNPC::CanInteract_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("[%s] CanInteract called by %s"),
		*GetName(),
		Interactor ? *Interactor->GetName() : TEXT("Unknown"));
	return true;
}

UAbilitySystemComponent* ACPBaseNPC::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FName ACPBaseNPC::GetPotionNPCId() const
{
	return IsValid(NPCData) ? NPCData->GetFName() : NAME_None;
}

UCPNPCSubsystem* ACPBaseNPC::GetNPCSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UCPNPCSubsystem>() : nullptr;
}

void ACPBaseNPC::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		//AbilitySystemComponent->OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &ACPBaseNPC::HandleGameplayEffectApplied);
		
		GrantPotionReactionAbilities();
	}

	if (UCPNPCSubsystem* Subsystem = GetNPCSubsystem())
	{
		Subsystem->OnNPCEffectApplied.AddDynamic(this, &ACPBaseNPC::HandlePotionEffectAppliedBroadcast);
		Subsystem->OnNPCEffectExpired.AddDynamic(this, &ACPBaseNPC::HandlePotionEffectExpiredBroadcast);
	}
	InitializeFromDataAsset();
	CatchUpPersistentPotionEffects();
}

void ACPBaseNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->OnGameplayEffectAppliedDelegateToSelf.RemoveAll(this);
	}

	if (UCPNPCSubsystem* Subsystem = GetNPCSubsystem())
	{
		Subsystem->OnNPCEffectApplied.RemoveDynamic(this, &ACPBaseNPC::HandlePotionEffectAppliedBroadcast);
		Subsystem->OnNPCEffectExpired.RemoveDynamic(this, &ACPBaseNPC::HandlePotionEffectExpiredBroadcast);
	}

	Super::EndPlay(EndPlayReason);
}

void ACPBaseNPC::InitializeFromDataAsset()
{
	if (!NPCData) { return; }

	USkeletalMesh* LoadedMesh = NPCData->NPCMesh.LoadSynchronous();
	if (LoadedMesh)
	{
		GetMesh()->SetSkeletalMesh(LoadedMesh);
		GetMesh()->SetRelativeScale3D(NPCData->MeshScale);
		BaseMeshScale = NPCData->MeshScale;
		FitCapsuleToMesh(LoadedMesh);
	}

	UAnimSequence* LoadedAnim = NPCData->IdleAnimation.LoadSynchronous();
	if (LoadedAnim)
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		GetMesh()->PlayAnimation(LoadedAnim, true); // 반복 재생
	}

	if (NPCData->NPCName != NAME_None)
	{
		Tags.AddUnique(NPCData->NPCName);
	}
}

void ACPBaseNPC::FitCapsuleToMesh(USkeletalMesh* InMesh)
{
	if (!InMesh || !GetMesh())
	{
		return;
	}

	const float CurrentScaleZ = GetMesh()->GetRelativeScale3D().Z;
	const FBoxSphereBounds MeshBounds = InMesh->GetBounds();
	const float MeshHalfHeight = MeshBounds.BoxExtent.Z * CurrentScaleZ;
	const float MeshRadius = MeshHalfHeight * (NPCData ? NPCData->CapsuleRadiusRatio : 0.3f);

	const float OldHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float ZOffset = MeshHalfHeight - OldHalfHeight;

	GetCapsuleComponent()->SetCapsuleSize(MeshRadius, MeshHalfHeight);
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -MeshHalfHeight));
	if (FMath::Abs(ZOffset) > KINDA_SMALL_NUMBER)
	{
		AddActorWorldOffset(FVector(0.f, 0.f, ZOffset), false);
	}

	GetCapsuleComponent()->SetCapsuleSize(MeshRadius, MeshHalfHeight);
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -MeshHalfHeight));
}

//void ACPBaseNPC::HandleGameplayEffectApplied(UAbilitySystemComponent* SourceASC, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveHandle)
//{
//}

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

void ACPBaseNPC::HandlePotionEffectAppliedBroadcast(FName BroadcastNPCId, FGameplayTag EffectTag, int64 ExpiresAtWorldMinute, float Magnitude)
{
	if (BroadcastNPCId != GetPotionNPCId())
	{
		return;
	}

	ApplyPotionEffectVisual(EffectTag, true, Magnitude);
}

void ACPBaseNPC::HandlePotionEffectExpiredBroadcast(FName BroadcastNPCId, FGameplayTag EffectTag)
{
	if (BroadcastNPCId != GetPotionNPCId())
	{
		return;
	}

	if (IsValid(AbilitySystemComponent) && PersistentPotionEffectTags.HasTagExact(EffectTag))
	{
		FGameplayEffectQuery Query;
		Query.OwningTagQuery = FGameplayTagQuery::MakeQuery_MatchAnyTags(FGameplayTagContainer(EffectTag));
		AbilitySystemComponent->RemoveActiveEffects(Query);
	}

	ApplyPotionEffectVisual(EffectTag, false, 1.0f);
}

void ACPBaseNPC::CatchUpPersistentPotionEffects()
{
	UCPNPCSubsystem* Subsystem = GetNPCSubsystem();
	const FName NPCId = GetPotionNPCId();
	if (!Subsystem || NPCId.IsNone())
	{
		return;
	}

	FCPNPCEffectSaveData SaveData;
	if (Subsystem->GetNPCEffectData(NPCId, SaveData))
	{
		for (const auto& EffectPair : SaveData.ActiveEffects)
		{
			ApplyPotionEffectVisual(EffectPair.Key, true, EffectPair.Value.Magnitude);
		}
	}
}

void ACPBaseNPC::ApplyPotionEffectVisual_Implementation(FGameplayTag EffectTag, bool bActive, float Magnitude)
{
	static const FGameplayTag GiantTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Giant"));

	if (EffectTag == GiantTag && GetMesh())
	{
		const FVector TargetScale = bActive ? (BaseMeshScale * Magnitude) : BaseMeshScale;
		GetMesh()->SetRelativeScale3D(TargetScale);
		FitCapsuleToMesh(GetMesh()->GetSkeletalMeshAsset());
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