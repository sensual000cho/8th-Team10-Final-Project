// GA_CPThrowProp.cpp

#include "Character/GAS/Abilities/GA_CPThrowProp.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Camera/PlayerCameraManager.h"
#include "Character/CPCarryComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

UGA_CPThrowProp::UGA_CPThrowProp() : ThrowSpeed(800.f), UpwardBias(0.2f)
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

    // Gameplay Event + Input Tag 시스템 두 가지로 실행 가능
    if (TriggerEventData && !TriggerEventData->EventTag.MatchesTagExact(GetThrowEventTag()))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    // 오직 Gameplay Event로만 GA를 실행시키려면 위의 조건문 대신 아래의 조건문으로 교체
    // if (!TriggerEventData || !TriggerEventData->EventTag.MatchesTagExact(GetThrowEventTag()))
    // {
    //     EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    //     return;
    // }

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

    const bool bThrowSucceeded = CarryComponent->ThrowHeldProp(ThrowDirection, ThrowSpeed);

    if (!bThrowSucceeded)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ThrowAbility] Prop 투척 실패"));

        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[ThrowAbility] Prop 투척 성공"));

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

float UGA_CPThrowProp::GetThrowSpeed() const
{
    return ThrowSpeed;
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