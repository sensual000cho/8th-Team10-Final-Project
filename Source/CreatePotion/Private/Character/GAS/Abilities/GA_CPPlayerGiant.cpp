#include "Character/GAS/Abilities/GA_CPPlayerGiant.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Character/CPCarryComponent.h"
#include "Components/ActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/CameraObjectInterface.h"
#include "Core/CameraRigAsset.h"
#include "Core/CameraVariableTable.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/BlueprintCameraEvaluationDataRef.h"
#include "GameFramework/GameplayCameraComponentBase.h"
#include "UObject/UnrealType.h"

UGA_CPPlayerGiant::UGA_CPPlayerGiant()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer InitialAssetTags;
	InitialAssetTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Reaction.Potion"))));
	// 기존 효과 태그로 중화 대상 목록에서 이 GA를 찾는다.
	InitialAssetTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Giant")));
	SetAssetTags(InitialAssetTags);

	EffectTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Effect.Giant")));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Immunity.Potion.All"))));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Immunity.Potion.Giant"))));
	ActivationBlockedTags.AddTag(EffectTag);
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("State.Reaction.Potion"))));
	ActivationOwnedTags.AddTag(EffectTag);

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Potion.Effect.Giant")));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UGA_CPPlayerGiant::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* PlayerCharacter = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!IsValid(PlayerCharacter) || DurationSeconds <= 0.0f || WalkSpeedMultiplier <= 0.0f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	CacheOriginalState(PlayerCharacter);
	ApplyGiantState();
	PlayerCharacter->GetWorldTimerManager().SetTimer(DurationTimerHandle, this, &UGA_CPPlayerGiant::HandleDurationExpired, DurationSeconds, false);
}

void UGA_CPPlayerGiant::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DurationTimerHandle);
	}

	RestoreOriginalState();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_CPPlayerGiant::HandleDurationExpired()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_CPPlayerGiant::HandlePreCharacterMovement()
{
	if (!bSpeedApplied)
	{
		CachedCharacterMovement->MaxWalkSpeed *= WalkSpeedMultiplier;
		bSpeedApplied = true;
	}
}

void UGA_CPPlayerGiant::RestoreCalculatedWalkSpeed(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	// Gust와 종료 순서가 달라도 Giant 배율만 제거한다.
	if (bSpeedApplied) CachedCharacterMovement->MaxWalkSpeed /= WalkSpeedMultiplier;
	bSpeedApplied = false;
}

void UGA_CPPlayerGiant::CacheOriginalState(ACharacter* PlayerCharacter)
{
	CachedCharacter = PlayerCharacter;
	CachedMesh = PlayerCharacter->GetMesh();
	CachedCapsule = PlayerCharacter->GetCapsuleComponent();
	CachedCarryComponent = PlayerCharacter->FindComponentByClass<UCPCarryComponent>();
	CachedCharacterMovement = PlayerCharacter->GetCharacterMovement();
	CachedGameplayCamera = PlayerCharacter->FindComponentByClass<UGameplayCameraComponentBase>();

	OriginalMeshScale = CachedMesh->GetRelativeScale3D();
	OriginalMeshLocation = CachedMesh->GetRelativeLocation();
	OriginalCapsuleRadius = CachedCapsule->GetUnscaledCapsuleRadius();
	OriginalCapsuleHalfHeight = CachedCapsule->GetUnscaledCapsuleHalfHeight();

	if (CachedCarryComponent.IsValid())
	{
		OriginalCarryLocation = CachedCarryComponent->GetRelativeLocation();
		OriginalThrowSpeedMultiplier = CachedCarryComponent->GetThrowSpeedMultiplier();
	}
}

void UGA_CPPlayerGiant::ApplyGiantState()
{
	CachedMesh->SetRelativeScale3D(OriginalMeshScale * Magnitude);
	CachedMesh->SetRelativeLocation(OriginalMeshLocation * Magnitude);

	CachedCapsule->SetCapsuleSize(OriginalCapsuleRadius * Magnitude, OriginalCapsuleHalfHeight * Magnitude, true);
	CachedCharacter->AddActorWorldOffset(FVector(0.0f, 0.0f, OriginalCapsuleHalfHeight * (Magnitude - 1.0f)), false, nullptr, ETeleportType::TeleportPhysics);
	BindPreCharacterMovement();
	CachedCharacter->OnCharacterMovementUpdated.AddUniqueDynamic(this, &UGA_CPPlayerGiant::RestoreCalculatedWalkSpeed);
	ApplyGiantCamera();

	if (CachedCarryComponent.IsValid())
	{
		CachedCarryComponent->SetRelativeLocation(OriginalCarryLocation * Magnitude);
		CachedCarryComponent->SetThrowSpeedMultiplier(OriginalThrowSpeedMultiplier * ThrowSpeedMultiplier);
	}

	bGiantApplied = true;
}

void UGA_CPPlayerGiant::RestoreOriginalState()
{
	if (!bGiantApplied || !CachedCharacter.IsValid())
	{
		return;
	}

	CachedMesh->SetRelativeScale3D(OriginalMeshScale);
	CachedMesh->SetRelativeLocation(OriginalMeshLocation);

	UnbindPreCharacterMovement();
	CachedCharacter->OnCharacterMovementUpdated.RemoveDynamic(this, &UGA_CPPlayerGiant::RestoreCalculatedWalkSpeed);
	CachedCapsule->SetCapsuleSize(OriginalCapsuleRadius, OriginalCapsuleHalfHeight, true);
	CachedCharacter->AddActorWorldOffset(FVector(0.0f, 0.0f, -OriginalCapsuleHalfHeight * (Magnitude - 1.0f)), false, nullptr, ETeleportType::TeleportPhysics);
	RestoreCalculatedWalkSpeed(0.0f, FVector::ZeroVector, FVector::ZeroVector);
	RestoreGiantCamera();

	if (CachedCarryComponent.IsValid())
	{
		CachedCarryComponent->SetRelativeLocation(OriginalCarryLocation);
		CachedCarryComponent->SetThrowSpeedMultiplier(OriginalThrowSpeedMultiplier);
	}

	bGiantApplied = false;
}

void UGA_CPPlayerGiant::BindPreCharacterMovement()
{
	const FObjectPropertyBase* PreCMCTickProperty = FindFProperty<FObjectPropertyBase>(CachedCharacter->GetClass(), TEXT("AC_PreCMCTick"));
	CachedPreCMCTickComponent = Cast<UActorComponent>(PreCMCTickProperty->GetObjectPropertyValue_InContainer(CachedCharacter.Get()));

	FMulticastDelegateProperty* TickDelegateProperty = FindFProperty<FMulticastDelegateProperty>(CachedPreCMCTickComponent->GetClass(), TEXT("Tick"));
	FScriptDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UGA_CPPlayerGiant, HandlePreCharacterMovement));
	TickDelegateProperty->AddDelegate(Delegate, CachedPreCMCTickComponent.Get(), TickDelegateProperty->ContainerPtrToValuePtr<void>(CachedPreCMCTickComponent.Get()));
}

void UGA_CPPlayerGiant::UnbindPreCharacterMovement()
{
	// 컴포넌트 제거 중에는 Tick 컴포넌트가 먼저 사라질 수 있다.
	if (!CachedPreCMCTickComponent.IsValid()) return;
	FMulticastDelegateProperty* TickDelegateProperty = FindFProperty<FMulticastDelegateProperty>(CachedPreCMCTickComponent->GetClass(), TEXT("Tick"));
	if (!TickDelegateProperty) return;
	FScriptDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UGA_CPPlayerGiant, HandlePreCharacterMovement));
	TickDelegateProperty->RemoveDelegate(Delegate, CachedPreCMCTickComponent.Get(), TickDelegateProperty->ContainerPtrToValuePtr<void>(CachedPreCMCTickComponent.Get()));
}

void UGA_CPPlayerGiant::ApplyGiantCamera()
{
	if (!CachedGameplayCamera.IsValid() || !IsValid(GiantCameraRig))
	{
		return;
	}

	const UCameraObjectInterfaceBlendableParameter* TranslationParameter = GiantCameraRig->Interface.FindBlendableParameterByName(TEXT("TranslationOffset"));
	const FBlueprintCameraEvaluationDataRef CameraData = CachedGameplayCamera->GetInitialResult();
	if (!IsValid(TranslationParameter) || !CameraData.IsValid())
	{
		return;
	}

	FCameraVariableDefinition VariableDefinition = TranslationParameter->GetVariableDefinition();
	VariableDefinition.bIsPrivate = false;
	CameraData.GetResult()->VariableTable.SetValue<FVector3d>(VariableDefinition, GiantCameraTranslationOffset, true);
	GiantCameraRigInstance = CachedGameplayCamera->StartGlobalCameraModifierRig(GiantCameraRig);
}

void UGA_CPPlayerGiant::RestoreGiantCamera()
{
	if (CachedGameplayCamera.IsValid() && GiantCameraRigInstance.IsValid())
	{
		CachedGameplayCamera->StopCameraModifierRig(GiantCameraRigInstance);
		GiantCameraRigInstance = FCameraRigInstanceID();
	}
}
