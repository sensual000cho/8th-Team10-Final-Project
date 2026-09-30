// GA_CPThrowProp.cpp

#include "Character/GAS/Abilities/GA_CPThrowProp.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Camera/PlayerCameraManager.h"
#include "Character/CPCarryComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

#include "Character/CPPlayerController.h"
#include "Components/CPInventoryComponent.h" 

UGA_CPThrowProp::UGA_CPThrowProp() : ThrowSpeed(800.f), UpwardBias(0.5f), UpwardThrowSpeedMultiplier(1.35f)
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    
    // 조준 상태에서만 Throw가 되도록
    // const FGameplayTag AimingStateTag = FGameplayTag::RequestGameplayTag(TEXT("State.Carry.Aiming"));
    // ActivationRequiredTags.AddTag(AimingStateTag);

    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = GetThrowEventTag();
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

    AbilityTriggers.Add(TriggerData);
}

bool UGA_CPThrowProp::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    const UCPCarryComponent* CarryComponent = FindCarryComponent(ActorInfo);

    if (!IsValid(CarryComponent))
    {
        return false;
    }

    return CarryComponent->HasHeldProp();
}

void UGA_CPThrowProp::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!ActorInfo)
    {
        return;
    }

    if (TriggerEventData &&
        !TriggerEventData->EventTag.MatchesTagExact(GetThrowEventTag()))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UCPCarryComponent* CarryComponent = FindCarryComponent(ActorInfo);

    if (!IsValid(CarryComponent) || !CarryComponent->HasHeldProp())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const FVector ThrowDirection = CalculateThrowDirection(ActorInfo->AvatarActor.Get(), ActorInfo->PlayerController.Get());

    if (ThrowDirection.IsNearlyZero())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    float ResolvedThrowSpeed = GetResolvedThrowSpeed(*CarryComponent);

    // 위쪽으로 조준할수록 속도를 추가로 올려서
    // 기본 중력 때문에 지나치게 무겁게 떨어지는 느낌을 완화.
    //
    // 수평/아래쪽 투척: VerticalAlpha = 0
    // 완전히 위쪽 투척: VerticalAlpha = 1
    const float VerticalAlpha = FMath::Clamp(ThrowDirection.Z, 0.f, 1.f);

    const float UpwardSpeedScale = FMath::Lerp(1.f, UpwardThrowSpeedMultiplier, VerticalAlpha);

    ResolvedThrowSpeed *= UpwardSpeedScale;

    const bool bThrowSucceeded = CarryComponent->ThrowHeldProp(ThrowDirection, ResolvedThrowSpeed);

    if (!bThrowSucceeded)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ThrowAbility] Prop 투척 실패"));

        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[ThrowAbility] Prop 투척 성공"));

    if (ACPPlayerController* PC = Cast<ACPPlayerController>(ActorInfo->PlayerController.Get()))
    {
        PC->ConsumeHeldPotionCharge();
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

float UGA_CPThrowProp::GetThrowSpeed() const
{
    return ThrowSpeed;
}

float UGA_CPThrowProp::GetResolvedThrowSpeed(const UCPCarryComponent& CarryComponent) const
{
    return ThrowSpeed * CarryComponent.GetThrowSpeedMultiplier();
}

UCPCarryComponent* UGA_CPThrowProp::FindCarryComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
    if (!ActorInfo)
    {
        return nullptr;
    }

    AActor* AvatarActor = ActorInfo->AvatarActor.Get();

    if (!IsValid(AvatarActor))
    {
        return nullptr;
    }

    return AvatarActor->FindComponentByClass<UCPCarryComponent>();
}

FVector UGA_CPThrowProp::CalculateThrowDirection(const AActor* AvatarActor, const APlayerController* PlayerController) const
{
    //매개변수 수정에 따른 방어코드 수정
    
    if (!IsValid(AvatarActor))
    {
        return FVector::ZeroVector;
    }

    FVector ForwardDirection = AvatarActor->GetActorForwardVector();

    if (IsValid(PlayerController) && IsValid(PlayerController->PlayerCameraManager))
    {
        ForwardDirection = PlayerController->PlayerCameraManager->GetCameraRotation().Vector();
    }

    return (ForwardDirection + FVector::UpVector * UpwardBias).GetSafeNormal();
}

const FGameplayTag& UGA_CPThrowProp::GetThrowEventTag()
{
    static const FGameplayTag ThrowEventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Carry.Throw")));
    return ThrowEventTag;
}
