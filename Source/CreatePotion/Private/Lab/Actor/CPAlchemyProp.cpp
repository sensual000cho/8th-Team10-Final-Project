#include "Lab/Actor/CPAlchemyProp.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/CPForageableItemData.h"
#include "GameState/CPLabGameState.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

ACPAlchemyProp::ACPAlchemyProp()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics;

    IngredientUprightPivot = CreateDefaultSubobject<USceneComponent>(TEXT("IngredientUprightPivot"));
    IngredientUprightPivot->SetupAttachment(StaticMeshComponent);

    IngredientBobblePivot = CreateDefaultSubobject<USceneComponent>(TEXT("IngredientBobblePivot"));
    IngredientBobblePivot->SetupAttachment(IngredientUprightPivot);

    IngredientMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IngredientMesh"));
    IngredientMeshComponent->SetupAttachment(IngredientBobblePivot);
    IngredientMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    IngredientMeshComponent->SetGenerateOverlapEvents(false);
    IngredientMeshComponent->SetSimulatePhysics(false);
    IngredientMeshComponent->SetCanEverAffectNavigation(false);
}

void ACPAlchemyProp::BeginPlay()
{
    Super::BeginPlay();

    const bool bHasPresentationMesh = IsValid(IngredientUprightPivot) && IsValid(IngredientBobblePivot) && IsValid(IngredientMeshComponent) && IsValid(IngredientMeshComponent->GetStaticMesh());
    SetActorTickEnabled(bEnableIngredientBobble && bHasPresentationMesh);

    if (!bHasPresentationMesh)
    {
        return;
    }

    IngredientBobbleBaseLocation = IngredientBobblePivot->GetRelativeLocation();
    BobbleElapsedTime = 0.f;
}

void ACPAlchemyProp::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateIngredientPresentation(DeltaSeconds);
}

void ACPAlchemyProp::UpdateIngredientPresentation(float DeltaSeconds)
{
    if (!bEnableIngredientBobble || !IsValid(IngredientUprightPivot) || !IsValid(IngredientBobblePivot))
    {
        return;
    }

    if (UprightRecoverySpeed > 0.f)
    {
        const FRotator CurrentRotation = IngredientUprightPivot->GetComponentRotation();
        const FRotator TargetRotation(0.f, CurrentRotation.Yaw, 0.f);
        const FRotator UprightRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, UprightRecoverySpeed);
        IngredientUprightPivot->SetWorldRotation(UprightRotation);
    }

    BobbleElapsedTime += DeltaSeconds * BobbleSpeed * 2.f * UE_PI;
    BobbleElapsedTime = FMath::Fmod(BobbleElapsedTime, 2.f * UE_PI);

    const float BobbleOffset = FMath::Sin(BobbleElapsedTime) * BobbleAmplitude;
    IngredientBobblePivot->SetRelativeLocation(IngredientBobbleBaseLocation + FVector::UpVector * BobbleOffset);
}

void ACPAlchemyProp::OnInteract_Implementation(AActor* Interactor)
{
    /* Legacy PotionSession 상호작용.
     *
     * 새로운 GAS 집기에서는 CPInteractionComponent가
     * Event.Carry.Pickup을 전달하므로 이 함수가 호출되지 않음.
     */
    UWorld* World = GetWorld();
    ACPLabGameState* LabGameState = World ? World->GetGameState<ACPLabGameState>() : nullptr;
    UCPLabPotionSessionComponent* Session = LabGameState ? LabGameState->GetPotionSession() : nullptr;

    if (!Session)
    {
        return;
    }

    // 기존에 들고 있던 재료가 있다면 플레이어 앞에 내려놓는다.
    ACPAlchemyProp* PreviousHeldProp = nullptr;

    if (Session->ReleaseHeldAlchemyProp(PreviousHeldProp) && IsValid(PreviousHeldProp))
    {
        FVector DropLocation = Interactor ? Interactor->GetActorForwardVector() * 100.f : FVector::ZeroVector;

        if (IsValid(Interactor))
        {
            DropLocation += Interactor->GetActorLocation();
        }

        PreviousHeldProp->SetActorLocation(DropLocation);
        PreviousHeldProp->SetActorHiddenInGame(false);
        PreviousHeldProp->SetActorEnableCollision(true);
    }

    // 기존 PotionSession 보유 방식
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

void ACPAlchemyProp::InitializeFromItemData(UCPForageableItemData* ItemData)
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

    if (EffectTags.Num() > 0)
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
