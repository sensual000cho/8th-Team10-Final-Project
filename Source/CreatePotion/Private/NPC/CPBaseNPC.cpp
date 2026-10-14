#include "NPC/CPBaseNPC.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "Data/NPC/CPGADataAsset.h"
#include "GameInstance/Subsystem/CPNPCSubsystem.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "NPC/CPNPCAIController.h"
#include "NPC/Components/CPNPCVisualComponent.h"
#include "NPC/Components/CPNPCMoveComponent.h"
#include "Character/CPInteractionComponent.h"

#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

ACPBaseNPC::ACPBaseNPC()
{
	PrimaryActorTick.bCanEverTick = true;
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
	MoveComponent = CreateDefaultSubobject<UCPNPCMoveComponent>(TEXT("MoveComponent"));

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ACPNPCAIController::StaticClass();
}

void ACPBaseNPC::SetHighlight_Implementation(bool bHighlighted)
{
	USkeletalMeshComponent* MainMesh = GetMesh();
	if (!MainMesh) { return; }

	if (bHighlighted)
	{
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (APawn* Pawn = PC->GetPawn())
				{
					if (UCPInteractionComponent* InteractionComp = Pawn->FindComponentByClass<UCPInteractionComponent>())
					{
						FProperty* Prop = UCPInteractionComponent::StaticClass()->FindPropertyByName(TEXT("HighlightMaterial"));
						if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop))
						{
							UMaterialInterface* HighlightMat = Cast<UMaterialInterface>(ObjProp->GetObjectPropertyValue_InContainer(InteractionComp));
							
							// Stencil 값
							static constexpr int32 HighlightStencilValue = 1;
							
							MainMesh->SetRenderCustomDepth(bHighlighted);
							MainMesh->SetCustomDepthStencilValue(bHighlighted ? HighlightStencilValue : 0);
							return;
						}
					}
				}
			}
		}
	}
	// MainMesh->SetOverlayMaterial(nullptr);
	MainMesh->SetRenderCustomDepth(false);
	MainMesh->SetCustomDepthStencilValue(0);
}

void ACPBaseNPC::PostInitializeComponents()
{
	Super::PostInitializeComponents();

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
	if (IsValid(AbilitySystemComponent)){
		AbilitySystemComponent->CancelAllAbilities();
	}
	
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

	if (GetMesh())
	{
		GetMesh()->SetRelativeRotation(NPCData->MeshRotationOffset);
	}

	if (VisualComponent)
	{
		VisualComponent->InitializeVisual(GetMesh(), GetCapsuleComponent(), LoadedMesh, NPCData->AnimClass, NPCData->MeshScale, NPCData->CapsuleRadiusRatio);
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

void ACPBaseNPC::SetNPCData(UCPNPCDataAsset* InNPCData)
{
	NPCData = InNPCData;
}

void ACPBaseNPC::SetNPCActiveState(bool bActive)
{
	SetActorHiddenInGame(!bActive);
    SetActorEnableCollision(bActive);
    SetActorTickEnabled(bActive);

    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->SetComponentTickEnabled(bActive);
        if (!bActive)
        {
            MovementComp->StopMovementImmediately();
        }
    }
    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        if (bActive)
        {
            AIC->ResumeMove(FAIRequestID::CurrentRequest);
        }
        else
        {
            AIC->StopMovement();
        }
    }

    if (MoveComponent)
    {
        if (bActive)
        {
            MoveComponent->StartMoving(); 
        }
        else
        {
            MoveComponent->StopMoving();
        }
    }
}

void ACPBaseNPC::CatchUpPersistentPotionEffects()
{
	UCPNPCSubsystem* Subsystem = GetNPCSubsystem();
	const FName NPCId = GetPotionNPCId();
	if (!Subsystem || NPCId.IsNone()) return;

	FCPNPCEffectSaveData SaveData;
	if (Subsystem->GetNPCEffectData(NPCId, SaveData))
	{
		bRestoringPersistentPotionEffects = true;
		for (const auto& EffectPair : SaveData.ActiveEffects)
		{
			if (AbilitySystemComponent)
			{
				AbilitySystemComponent->AddLooseGameplayTag(EffectPair.Key);
			}
			ApplyPotionEffectVisual(EffectPair.Key, true, EffectPair.Value.Magnitude);
		}
		
		bRestoringPersistentPotionEffects = false;
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

void ACPBaseNPC::RegisterCodex()
{
	if (!NPCData) return;
	
	UGameInstance* GameInstance = GetGameInstance();
	UCPCodexSubsystem* CodexSubsystem = GameInstance ? GameInstance->GetSubsystem<UCPCodexSubsystem>() : nullptr;
	if (!CodexSubsystem) return;
	
	CodexSubsystem->RegisterNPCEntry(NPCData);
}

#pragma region Route Debug

void ACPBaseNPC::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	UWorld* World = GetWorld();
	if (World && !World->IsGameWorld())
	{
		if (bShowPatrolRouteInEditor)
		{
			DrawDebugPatrolRoute();
		}
		else
		{
			FlushPersistentDebugLines(World);
		}
	}
#endif
}

#if WITH_EDITOR
void ACPBaseNPC::DrawDebugPatrolRoute()
{
	UWorld* World = GetWorld();
	if (!World || World->IsGameWorld())
	{
		return;
	}

	FlushPersistentDebugLines(World);

	if (!NPCData)
	{
		return;
	}
	FString CurrentLevelName;
	if (ULevel* OwnerLevel = GetLevel())
	{
		CurrentLevelName = FPackageName::GetShortName(OwnerLevel->GetPackage());
	}
	CurrentLevelName = UWorld::RemovePIEPrefix(CurrentLevelName);

	const FCPNPCRoute* SelectedRoute = nullptr;
	if (const FCPNPCTargetRow* Row = NPCData->TargetRowHandle.GetRow<FCPNPCTargetRow>(TEXT("DrawDebugPatrolRoute")))
	{
		for (const FCPNPCRoute& Route : Row->Routes)
		{
			if (Route.TargetLevel.IsValid() || !Route.TargetLevel.IsNull())
			{
				if (Route.TargetLevel.GetAssetName().Equals(CurrentLevelName, ESearchCase::IgnoreCase))
				{
					SelectedRoute = &Route;
					break;
				}
			}
		}
	}

	if (!SelectedRoute || SelectedRoute->TargetPoints.Num() == 0)
	{
		return;
	}

	const TArray<FCPTargetPoint>& Points = SelectedRoute->TargetPoints;
	const int32 NumPoints = Points.Num();

	DrawDebugLine(World, GetActorLocation(), Points[0].Point, FColor::Yellow, true, -1.f, 0, RouteLineThickness);

	for (int32 i = 0; i < NumPoints; ++i)
	{
		const FVector& CurrentPoint = Points[i].Point;
		const float WaitTime = Points[i].WaitSeconds;

		DrawDebugSphere(World, CurrentPoint, 35.0f, 16, PatrolRouteColor, true, -1.f, 0, 2.5f);

		const FString PointLabel = FString::Printf(
			TEXT("Point [%d] (%.1fs)\nWorld: (%.0f, %.0f, %.0f)"),
			i, WaitTime, CurrentPoint.X, CurrentPoint.Y, CurrentPoint.Z
		);
		DrawDebugString(World, CurrentPoint + FVector(0.f, 0.f, 55.f), PointLabel, nullptr, FColor::White, -1.f, true, 1.2f);

		if (i + 1 < NumPoints)
		{
			DrawDebugDirectionalArrow(World, CurrentPoint, Points[i + 1].Point, 70.0f, PatrolRouteColor, true, -1.f, 0, RouteLineThickness);
		}
		else if (SelectedRoute->bLoop && NumPoints > 1)
		{
			DrawDebugDirectionalArrow(World, CurrentPoint, Points[0].Point, 70.0f, FColor::Silver, true, -1.f, 0, RouteLineThickness);
		}
	}
}
#endif

#pragma endregion Route Debug
