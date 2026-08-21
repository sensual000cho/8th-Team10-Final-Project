// CPCarryComponent.cpp

#include "Character/CPCarryComponent.h"

#include "GameState/CPLabGameState.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Actor/CPThrowablePropBase.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

UCPCarryComponent::UCPCarryComponent() : ResetDropForwardDistance(100.f)
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
    
    return IsValid(Prop) && !HasHeldProp();
}

bool UCPCarryComponent::AttachProp(ACPThrowablePropBase* Prop)
{
    if (!CanAttachProp(Prop))
    {
        return false;
    }

    // 동일한 Prop이 이미 정상적으로 등록되어 있으면 성공으로 처리
    if (HeldProp == Prop)
    {
        return true;
    }
    
    // 다른 Prop을 들고 있다면 새 Prop을 집을 수 없음
    // 이 부분은 GA에서 다른 Prop으로 교체하게 추후에 수정예정
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

    return DetachProp(PropToDrop, DropLocation);
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

/*
 * -------------------------------------------------------------------------
 * Legacy / PotionSession 전용
 *
 * 새로운 Carry 시스템 및 GA에는 사용 X
 * 기존 BP나 세션이 바뀌면 제거 예정.
 * -------------------------------------------------------------------------
 */
bool UCPCarryComponent::TryThrowHeldAlchemyProp(float ThrowSpeed, float UpwardBias)
{
    AActor* OwnerActor = GetOwner();

    if (!IsValid(OwnerActor) || !HasHeldProp())
    {
        return false;
    }

    // 기존 세션 시스템은 연금술 재료만 처리
    ACPAlchemyProp* HeldAlchemyProp = Cast<ACPAlchemyProp>(HeldProp);

    if (!IsValid(HeldAlchemyProp))
    {
        return false;
    }

    UWorld* World = GetWorld();

    ACPLabGameState* LabGameState = World ? World->GetGameState<ACPLabGameState>() : nullptr;

    UCPLabPotionSessionComponent* PotionSession = LabGameState ? LabGameState->GetPotionSession() : nullptr;

    if (!PotionSession)
    {
        return false;
    }

    // 기존 세션 규칙: Processing 단계에서만 재료 투척 가능
    if (!PotionSession->CanThrowHeldAlchemyProp())
    {
        return false;
    }

    // CarryComponent와 PotionSession이 같은 재료를 보관하는지 확인
    if (PotionSession->GetHeldAlchemyProp() != HeldAlchemyProp)
    {
        return false;
    }

    const FVector ThrowDirection = (OwnerActor->GetActorForwardVector() + FVector::UpVector * UpwardBias).GetSafeNormal();

    ACPAlchemyProp* ReleasedProp = nullptr;

    // 기존 PotionSession에서 Held 참조 해제
    if (!PotionSession->ReleaseHeldAlchemyProp(ReleasedProp))
    {
        return false;
    }

    if (ReleasedProp != HeldAlchemyProp)
    {
        // 세션 참조 불일치가 발생했다면 기존 상태로 복구
        if (IsValid(ReleasedProp))
        {
            PotionSession->HoldAlchemyProp(ReleasedProp);
        }

        return false;
    }

    if (!ThrowHeldProp(ThrowDirection, ThrowSpeed))
    {
        // 실제 투척에 실패했다면 세션 Held 참조 복구
        PotionSession->HoldAlchemyProp(HeldAlchemyProp);
        return false;
    }

    return true;
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
