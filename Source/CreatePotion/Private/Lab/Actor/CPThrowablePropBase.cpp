#include "Lab/Actor/CPThrowablePropBase.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

ACPThrowablePropBase::ACPThrowablePropBase()
{
    PrimaryActorTick.bCanEverTick = false;

    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(StaticMeshComponent);
}

bool ACPThrowablePropBase::AttachAsHeld(
    USceneComponent* CarryAnchor)
{
    if (!IsValid(CarryAnchor) || !IsValid(StaticMeshComponent))
    {
        return false;
    }

    // 기존 물리 운동 제거
    StaticMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
    StaticMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    StaticMeshComponent->SetSimulatePhysics(false);
    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    const bool bAttached = AttachToComponent(CarryAnchor, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

    if (!bAttached)
    {
        StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        return false;
    }

    SetActorHiddenInGame(false);
    SetActorRelativeTransform(HeldRelativeTransform);

    return true;
}

void ACPThrowablePropBase::DetachAsHeld(const FVector& DropLocation)
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    SetActorLocation(
        DropLocation,
        false,
        nullptr,
        ETeleportType::TeleportPhysics);

    SetActorHiddenInGame(false);
    StaticMeshComponent->SetSimulatePhysics(false);
    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StaticMeshComponent->SetGenerateOverlapEvents(true);
}

bool ACPThrowablePropBase::Throw(const FVector& Direction, float Speed)
{
    if (!IsValid(StaticMeshComponent) || Direction.IsNearlyZero() || Speed <= 0.f)
    {
        return false;
    }

    const FVector ThrowDirection = Direction.GetSafeNormal();

    // 현재 머리 위 위치를 유지하면서 캐릭터에서 분리
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    /*
     * 물리를 켜기 전에 투척 방향으로 조금 이동한다.
     * 캐릭터 캡슐 또는 머리와 즉시 충돌하는 것을 줄여준다.
     */
    SetActorLocation(
        GetActorLocation() +
            ThrowDirection * ThrowStartOffset,
        false,
        nullptr,
        ETeleportType::TeleportPhysics);

    SetActorHiddenInGame(false);

    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StaticMeshComponent->SetGenerateOverlapEvents(true);
    StaticMeshComponent->SetSimulatePhysics(true);
    StaticMeshComponent->WakeAllRigidBodies();
    StaticMeshComponent->SetPhysicsLinearVelocity(ThrowDirection * Speed);

    return true;
}

bool ACPThrowablePropBase::CanInteract_Implementation(AActor* Interactor)
{
    return IsValid(Interactor);
}

FName ACPThrowablePropBase::GetInteractionName_Implementation()
{
    return FName(TEXT("Prop"));
}