#include "Lab/Actor/CPAlchemyProp.h"

#include "Data/CPForageableItemData.h"
#include "GameState/CPLabGameState.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

void ACPAlchemyProp::OnInteract_Implementation(AActor* Interactor)
{
    if (!IsValid(Interactor))
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

    // 기존에 들고 있던 재료가 있다면 캐릭터 앞에 내려놓음
    ACPAlchemyProp* PreviousHeldProp = nullptr;

    if (Session->ReleaseHeldAlchemyProp(PreviousHeldProp) && IsValid(PreviousHeldProp))
    {
        const FVector DropLocation = Interactor->GetActorLocation() + Interactor->GetActorForwardVector() * 100.f;

        PreviousHeldProp->SetActorLocation(DropLocation);
        PreviousHeldProp->SetActorHiddenInGame(false);
        PreviousHeldProp->SetActorEnableCollision(true);
    }

    // 상호작용한 재료를 현재 Held 재료로 등록
    if (!Session->HoldAlchemyProp(this))
    {
        return;
    }

    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
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
