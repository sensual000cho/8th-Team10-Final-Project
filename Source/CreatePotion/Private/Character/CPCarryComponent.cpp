// CPCarryComponent.cpp

#include "Character/CPCarryComponent.h"

#include "GameState/CPLabGameState.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Actor/CPThrowablePropBase.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

UCPCarryComponent::UCPCarryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UCPCarryComponent::AttachProp(ACPThrowablePropBase* Prop)
{
    if (!IsValid(Prop))
    {
        return false;
    }

    // 유효한 다른 Prop을 이미 들고 있다면 부착 불가
    if (HasHeldProp() && HeldProp != Prop)
    {
        return false;
    }

    if (!Prop->AttachAsHeld(this))
    {
        return false;
    }

    HeldProp = Prop;
    return true;
}

bool UCPCarryComponent::DetachProp(ACPThrowablePropBase* Prop, const FVector& DropLocation)
{
    if (!IsValid(Prop))
    {
        return false;
    }

    Prop->DetachAsHeld(DropLocation);

    if (HeldProp == Prop)
    {
        HeldProp = nullptr;
    }

    return true;
}

bool UCPCarryComponent::HasHeldProp() const
{
    return IsValid(HeldProp);
}

ACPThrowablePropBase* UCPCarryComponent::GetHeldProp() const
{
    return IsValid(HeldProp) ? HeldProp.Get() : nullptr;
}

bool UCPCarryComponent::TryThrowHeldAlchemyProp(float ThrowSpeed, float UpwardBias)
{
    AActor* OwnerActor = GetOwner();

    if (!IsValid(OwnerActor) || !HasHeldProp())
    {
        return false;
    }

    // 현재 Held Prop이 실제 연금술 재료인지 확인
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

    // 재료는 Processing 단계에서만 투척 가능
    if (!PotionSession->CanThrowHeldAlchemyProp())
    {
        return false;
    }

    /*
     * CarryComponent와 PotionSession이 같은 재료를
     * 들고 있는지 확인한다.
     */
    if (PotionSession->GetHeldAlchemyProp() != HeldAlchemyProp)
    {
        return false;
    }

    const FVector ThrowDirection = (OwnerActor->GetActorForwardVector() + FVector::UpVector * UpwardBias).GetSafeNormal();

    ACPAlchemyProp* ReleasedProp = nullptr;

    // 투척 순간 Session에서 Held 참조 해제
    if (!PotionSession->ReleaseHeldAlchemyProp(ReleasedProp))
    {
        return false;
    }

    if (ReleasedProp != HeldAlchemyProp)
    {
        // 예외적인 상태 불일치가 발생했다면 복구
        if (IsValid(ReleasedProp))
        {
            PotionSession->HoldAlchemyProp(ReleasedProp);
        }

        return false;
    }

    if (!ThrowHeldProp(ThrowDirection, ThrowSpeed))
    {
        /*
         * Throw()는 검증 실패 시 Actor 상태를 변경하지 않으므로
         * Session 참조만 다시 복구하면 된다.
         */
        PotionSession->HoldAlchemyProp(HeldAlchemyProp);
        return false;
    }

    return true;
}

bool UCPCarryComponent::ThrowHeldProp(const FVector& Direction, float Speed)
{
    ACPThrowablePropBase* PropToThrow = GetHeldProp();

    if (!IsValid(PropToThrow))
    {
        return false;
    }

    if (!PropToThrow->Throw(Direction, Speed))
    {
        return false;
    }

    // 투척 성공 후에만 Held 참조 해제
    HeldProp = nullptr;

    return true;
}

