// CPCarryComponent.cpp

#include "Character/CPCarryComponent.h"

#include "Lab/Actor/CPThrowablePropBase.h"

UCPCarryComponent::UCPCarryComponent()  
    : ReplacementDropTraceUpDistance(100.f),
      ReplacementDropTraceDownDistance(300.f),
      ReplacementDropGroundClearance(3.f),
      ResetDropForwardDistance(100.f)
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

    return !HasHeldProp() && Prop->CanBePickedUp();
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
    if (!NewProp->CanBePickedUp())
    {
        return false;
    }

    if (IsValid(PreviousHeldProp))
    {
        // NewProp이 부착되기 전에 원래 위치 기준으로 계산
        const FVector DropLocation = MakeReplacementDropLocation(PreviousHeldProp, NewProp);

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

FVector UCPCarryComponent::MakeReplacementDropLocation(const ACPThrowablePropBase* PreviousHeldProp, const ACPThrowablePropBase* NewProp) const
{
    if (!IsValid(PreviousHeldProp) || !IsValid(NewProp))
    {
        return GetComponentLocation();
    }

    FVector PreviousBoundsOrigin;
    FVector PreviousBoundsExtent;

    if (!PreviousHeldProp->GetPropCollisionBounds(PreviousBoundsOrigin, PreviousBoundsExtent))
    {
        return NewProp->GetActorLocation();
    }

    FVector NewBoundsOrigin;
    FVector NewBoundsExtent;

    if (!NewProp->GetPropCollisionBounds(NewBoundsOrigin, NewBoundsExtent))
    {
        return NewProp->GetActorLocation();
    }

    const FVector PreviousActorLocation = PreviousHeldProp->GetActorLocation();

    // 기존 Held Prop의 피벗에서 Bounds 중심까지의 차이
    const FVector PreviousBoundsOffset = PreviousBoundsOrigin - PreviousActorLocation;

    FVector DropLocation;

    // 기존 Held Prop의 Bounds 중심을 NewProp의 기존 Bounds 중심에 맞춤
    DropLocation.X = NewBoundsOrigin.X - PreviousBoundsOffset.X;
    DropLocation.Y = NewBoundsOrigin.Y - PreviousBoundsOffset.Y;

    // NewProp이 놓여 있던 바닥 높이
    const float TargetFloorZ = NewBoundsOrigin.Z - NewBoundsExtent.Z;

    // 기존 Held Prop의 피벗에서 Bounds 바닥까지의 거리
    const float PreviousBoundsBottomZ = PreviousBoundsOrigin.Z - PreviousBoundsExtent.Z;
    const float PreviousBottomOffset = PreviousActorLocation.Z - PreviousBoundsBottomZ;

    // 기존 Prop의 Bounds 바닥이 원래 NewProp의 바닥 위치에 오도록 배치
    DropLocation.Z = TargetFloorZ + PreviousBottomOffset + ReplacementDropClearance;

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
