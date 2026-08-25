// GA_CPPickupProp.cpp

#include "Character/GAS/Abilities/GA_CPPickupProp.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Character/CPCarryComponent.h"
#include "GameFramework/Actor.h"
#include "Lab/Actor/CPThrowablePropBase.h"

UGA_CPPickupProp::UGA_CPPickupProp()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = GetPickupEventTag();
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

    AbilityTriggers.Add(TriggerData);
}

bool UGA_CPPickupProp::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
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

    // 한 번에 하나만 들 수 있으므로 이미 들고 있다면 활성화 불가
    return !CarryComponent->HasHeldProp();
}

void UGA_CPPickupProp::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!ActorInfo || !TriggerEventData)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!TriggerEventData->EventTag.MatchesTagExact(GetPickupEventTag()))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UCPCarryComponent* CarryComponent = FindCarryComponent(ActorInfo);

    if (!IsValid(CarryComponent))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // FGameplayEventData::Target은 const AActor 포인터라서 Prop 조작을 위해 원래 포인터로 변환한다.
    AActor* TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());
    ACPThrowablePropBase* TargetProp = Cast<ACPThrowablePropBase>(TargetActor);

    if (!IsValid(TargetProp))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const bool bPickupSucceeded = CarryComponent->AttachProp(TargetProp);

    if (!bPickupSucceeded)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PickupAbility] Prop 부착 실패: %s"), *GetNameSafe(TargetProp));

        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[PickupAbility] Prop 부착 성공: %s"), *GetNameSafe(TargetProp));

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

UCPCarryComponent* UGA_CPPickupProp::FindCarryComponent(const FGameplayAbilityActorInfo* ActorInfo) const
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

const FGameplayTag& UGA_CPPickupProp::GetPickupEventTag()
{
    static const FGameplayTag PickupEventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Carry.Pickup")));
    return PickupEventTag;
}
