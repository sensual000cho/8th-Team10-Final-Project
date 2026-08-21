#include "Lab/Actor/CPAlchemyProp.h"

#include "Data/CPForageableItemData.h"
#include "GameState/CPLabGameState.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"
#include "Character/CPCarryComponent.h"

void ACPAlchemyProp::OnInteract_Implementation(AActor* Interactor)
{
    if (!IsValid(Interactor))
    {
        return;
    }

    UCPCarryComponent* CarryComponent = Interactor->FindComponentByClass<UCPCarryComponent>();

    if (!CarryComponent)
    {
        return;
    }

    UWorld* World = GetWorld();

    ACPLabGameState* LabGameState = World ? World->GetGameState<ACPLabGameState>() : nullptr;

    UCPLabPotionSessionComponent* Session = LabGameState ? LabGameState->GetPotionSession() : nullptr;

    if (!Session)
    {
        return;
    }

    // 이전에 들고 있던 재료가 있다면 내려놓음
    ACPAlchemyProp* PreviousHeldProp = nullptr;

    if (Session->ReleaseHeldAlchemyProp(PreviousHeldProp) && IsValid(PreviousHeldProp))
    {
        const FVector DropLocation = Interactor->GetActorLocation() + Interactor->GetActorForwardVector() * 100.f;

        CarryComponent->DetachProp(PreviousHeldProp, DropLocation);
    }

    // 새 재료를 Session에 등록
    if (!Session->HoldAlchemyProp(this))
    {
        return;
    }

    // 머리 위 CarryComponent에 부착
    if (!CarryComponent->AttachProp(this))
    {
        // 부착에 실패했다면 Session 변경도 롤백
        ACPAlchemyProp* ReleasedProp = nullptr;
        Session->ReleaseHeldAlchemyProp(ReleasedProp);
    }
}

FText ACPAlchemyProp::GetInteractionPrompt_Implementation()
{
    return FText::FromString(TEXT("재료 들기"));
}

void ACPAlchemyProp::InitializeFromItemData(
    UCPForageableItemData* ItemData)
{
    InitializeAlchemyProp(ItemData);
}

void ACPAlchemyProp::InitializeAlchemyProp(UCPForageableItemData* ItemData, const TArray<FGameplayTag>& EffectTags)
{
    WorkingIngredient = FCPLabIngredientInstance{};

    if (!ItemData)
    {
        return;
    }

    WorkingIngredient.SourceItemData = ItemData;

    if (!EffectTags.IsEmpty())
    {
        WorkingIngredient.CurrentEffects = EffectTags;
    }
    else
    {
        WorkingIngredient.CurrentEffects = ItemData->TagAxes;
    }
}

FCPLabIngredientInstance ACPAlchemyProp::GetWorkingIngredient() const
{
    return WorkingIngredient;
}
