// CPCarryComponent.cpp

#include "Character/CPCarryComponent.h"

#include "Lab/Actor/CPThrowablePropBase.h"

UCPCarryComponent::UCPCarryComponent() : ReplacementDropForwardDistance(120.f), ResetDropForwardDistance(100.f)
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCPCarryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    /*
     * 레벨 전환이나 World 종료시 물리 상태를 다시 변경하지 않고
     * 델리게이트와 참조만 안전하게 해제.
     */
    if (IsValid(HeldProp))
    {
        HeldProp->OnDestroyed.RemoveDynamic(this, &UCPCarryComponent::HandleHeldPropDestroyed);
    }

    HeldProp = nullptr;

    Super::EndPlay(EndPlayReason);
}

bool UCPCarryComponent::CanAttachProp(ACPThrowablePropBase* Prop) const
{
    if (!IsValid(Prop))
    {
        return false;
    }
    
    if (HeldProp == Prop)
    {
        return Prop->IsHeld();
    }

    return !HasHeldProp() && Prop->IsResting();
}

bool UCPCarryComponent::AttachProp(ACPThrowablePropBase* Prop)
{
    if (!IsValid(Prop))
    {
        return false;
    }
    
    if (HeldProp == Prop)
    {
        return Prop->IsHeld();
    }
    
    // 다른 Prop을 들고 있다면 새 Prop을 집을 수 없음
    // 이 부분은 GA에서 다른 Prop으로 교체하게 추후에 수정예정
    // -> ReplaceHeldProp으로 기능을 뺐음
    if (!CanAttachProp(Prop))
    {
        return false;
    }

    if (!Prop->AttachAsHeld(this))
    {
        return false;
    }

    SetHeldProp(Prop);
    return true;
}

bool UCPCarryComponent::ReplaceHeldProp(ACPThrowablePropBase* NewProp)
{
    if (!IsValid(NewProp))
    {
        return false;
    }

    ACPThrowablePropBase* PreviousHeldProp = GetHeldProp();

    if (PreviousHeldProp == NewProp)
    {
        return NewProp->IsHeld();
    }

    // 날아가거나 Drop 중인 Prop으로 교체할 수 없음
    if (!NewProp->IsResting())
    {
        return false;
    }

    if (IsValid(PreviousHeldProp))
    {
        const FVector DropLocation = MakeReplacementDropLocation();

        if (!DropHeldProp(DropLocation))
        {
            return false;
        }
    }
    
    return AttachProp(NewProp);
}

bool UCPCarryComponent::DetachProp(ACPThrowablePropBase* Prop, const FVector& DropLocation)
{
    if (!IsValid(Prop))
    {
        return false;
    }

    // 다른 CarryComponent나 외부 시스템이 가진 Prop을 잘못 분리하지 않도록 검사
    if (GetHeldProp() != Prop)
    {
        return false;
    }

    Prop->DetachAsHeld(DropLocation);
    SetHeldProp(nullptr);

    return true;
}

bool UCPCarryComponent::DropHeldProp(const FVector& DropLocation)
{
    ACPThrowablePropBase* PropToDrop = GetHeldProp();

    if (!IsValid(PropToDrop))
    {
        return false;
    }

    if (!PropToDrop->Drop(DropLocation))
    {
        return false;
    }

    SetHeldProp(nullptr);
    return true;
}

bool UCPCarryComponent::ThrowHeldProp(const FVector& Direction, float Speed)
{
    ACPThrowablePropBase* PropToThrow = GetHeldProp();

    if (!IsValid(PropToThrow))
    {
        return false;
    }

    if (Direction.IsNearlyZero() || Speed <= 0.f)
    {
        return false;
    }

    const FVector NormalizedDirection = Direction.GetSafeNormal();

    if (!PropToThrow->Throw(NormalizedDirection, Speed))
    {
        return false;
    }

    // 실제 투척에 성공한 이후에만 Held 참조를 해제
    SetHeldProp(nullptr);

    return true;
}

void UCPCarryComponent::ResetCarryState()
{
    ACPThrowablePropBase* PropToRelease = GetHeldProp();

    if (!IsValid(PropToRelease))
    {
        SetHeldProp(nullptr);
        return;
    }

    FVector DropLocation = GetComponentLocation();

    const AActor* OwnerActor = GetOwner();

    if (IsValid(OwnerActor))
    {
        DropLocation = OwnerActor->GetActorLocation();
        DropLocation += OwnerActor->GetActorForwardVector() * ResetDropForwardDistance;
    }

    PropToRelease->DetachAsHeld(DropLocation);
    SetHeldProp(nullptr);
}

bool UCPCarryComponent::HasHeldProp() const
{
    return IsValid(HeldProp);
}

ACPThrowablePropBase* UCPCarryComponent::GetHeldProp() const
{
    return IsValid(HeldProp) ? HeldProp.Get() : nullptr;
}

FVector UCPCarryComponent::MakeReplacementDropLocation() const
{
    FVector DropLocation = GetComponentLocation();

    const AActor* OwnerActor = GetOwner();

    if (IsValid(OwnerActor))
    {
        DropLocation += OwnerActor->GetActorForwardVector() * ReplacementDropForwardDistance;
    }

    return DropLocation;
}

void UCPCarryComponent::SetHeldProp(ACPThrowablePropBase* NewHeldProp)
{
    if (HeldProp.Get() == NewHeldProp)
    {
        return;
    }

    if (IsValid(HeldProp))
    {
        HeldProp->OnDestroyed.RemoveDynamic(this, &UCPCarryComponent::HandleHeldPropDestroyed);
    }

    HeldProp = NewHeldProp;

    if (IsValid(HeldProp))
    {
        HeldProp->OnDestroyed.AddUniqueDynamic(this, &UCPCarryComponent::HandleHeldPropDestroyed);
    }

    OnHeldPropChanged.Broadcast(HeldProp.Get());
}

void UCPCarryComponent::HandleHeldPropDestroyed(AActor* DestroyedActor)
{
    if (DestroyedActor != HeldProp.Get())
    {
        return;
    }

    SetHeldProp(nullptr);
}
