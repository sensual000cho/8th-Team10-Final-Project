// GA_CPAimThrowProp.cpp


#include "Character/GAS/Abilities/GA_CPAimThrowProp.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Character/CPCarryComponent.h"
#include "Lab/Actor/CPThrowablePropBase.h"

UGA_CPAimThrowProp::UGA_CPAimThrowProp()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    
    // Aim Pressed/Released일 때 Action Value의 bool값에 따라 Event를 보냄.
    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = GetAimStartEventTag();
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(TriggerData);

    // 이 Ability를 구분하기 위한 태그
    AbilityTags.AddTag(GetAimAbilityTag());
    
    // Ability 활성화 동안 Tag 부여(아래의 Tag가 있어야 Throw GA를 실행할 수 있음)
    ActivationOwnedTags.AddTag(GetAimingStateTag());
}

bool UGA_CPAimThrowProp::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        return false;
    }

    const AActor* AvatarActor = ActorInfo->AvatarActor.Get();
    const UCPCarryComponent* CarryComponent = AvatarActor->FindComponentByClass<UCPCarryComponent>();

    if (!IsValid(CarryComponent) || !CarryComponent->HasHeldProp())
    {
        return false;
    }

    const UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();

    if (!IsValid(AbilitySystemComponent))
    {
        return false;
    }

    // 이미 Aim 상태라면 실행 X
    if (AbilitySystemComponent->HasMatchingGameplayTag(GetAimingStateTag()))
    {
        return false;
    }

    return true;
}

void UGA_CPAimThrowProp::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // Gameplay Event로 Activate된 경우 Tag를 꼼꼼히 확인
    if (TriggerEventData && !TriggerEventData->EventTag.MatchesTagExact(GetAimStartEventTag()))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;

    if (!IsValid(AvatarActor))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    BoundCarryComponent = AvatarActor->FindComponentByClass<UCPCarryComponent>();

    if (!IsValid(BoundCarryComponent) || !BoundCarryComponent->HasHeldProp())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 투척 혹은 투척물이 사라지는 상황에서 어빌리티 종료
    BoundCarryComponent->OnHeldPropChanged.AddUniqueDynamic(this, &UGA_CPAimThrowProp::HandleHeldPropChanged);

    // 우클릭이 끝나는 상황(Event.Carry.Aim.End)을 기다림
    WaitAimEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        GetAimEndEventTag(),
        nullptr,
        true,
        true
    );

    if (!IsValid(WaitAimEndTask))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    WaitAimEndTask->EventReceived.AddDynamic(this, &UGA_CPAimThrowProp::HandleAimEndEvent);
    WaitAimEndTask->ReadyForActivation();

    // TODO: CPTrajectoryPreviewComponent의 ActivatePreview()를 호출
    
    UE_LOG(LogTemp, Log, TEXT("[AimThrowAbility] 투척 조준 시작"));
}

void UGA_CPAimThrowProp::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    // TODO: CPTrajectoryPreviewComponent의 DeactivatePreview()를 호출


    if (IsValid(BoundCarryComponent))
    {
        BoundCarryComponent->OnHeldPropChanged.RemoveDynamic(this, &UGA_CPAimThrowProp::HandleHeldPropChanged);
    }

    BoundCarryComponent = nullptr;

    if (IsValid(WaitAimEndTask))
    {
        WaitAimEndTask->EventReceived.RemoveDynamic(this, &UGA_CPAimThrowProp::HandleAimEndEvent);
        WaitAimEndTask->EndTask();
    }

    WaitAimEndTask = nullptr;

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[AimThrowAbility] 투척 조준 종료: %s"),
        bWasCancelled ? TEXT("취소") : TEXT("정상 종료")
    );

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_CPAimThrowProp::HandleAimEndEvent(FGameplayEventData Payload)
{
    if (!Payload.EventTag.MatchesTagExact(GetAimEndEventTag()))
    {
        return;
    }

    EndCurrentAim(false);
}

void UGA_CPAimThrowProp::HandleHeldPropChanged(ACPThrowablePropBase* NewHeldProp)
{
    if (IsValid(NewHeldProp))
    {
        return;
    }

    // 투척·리셋·파괴 등으로 HeldProp이 사라지면 Aim 취소
    EndCurrentAim(true);
}

void UGA_CPAimThrowProp::EndCurrentAim(bool bWasCancelled)
{
    if (!IsActive() || !CurrentActorInfo)
    {
        return;
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

const FGameplayTag& UGA_CPAimThrowProp::GetAimStartEventTag()
{
    static const FGameplayTag AimStartEventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Carry.Aim.Start")));
    return AimStartEventTag;
}

const FGameplayTag& UGA_CPAimThrowProp::GetAimEndEventTag()
{
    static const FGameplayTag AimEndEventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Carry.Aim.End")));
    return AimEndEventTag;
}

const FGameplayTag& UGA_CPAimThrowProp::GetAimingStateTag()
{
    static const FGameplayTag AimingStateTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Carry.Aiming")));
    return AimingStateTag;
}

const FGameplayTag& UGA_CPAimThrowProp::GetAimAbilityTag()
{
    static const FGameplayTag AimAbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Carry.Aim")));
    return AimAbilityTag;
}