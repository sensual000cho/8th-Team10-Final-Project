#include "Lab/Actor/CPThrowablePropBase.h"
#include "Components/StaticMeshComponent.h"

ACPThrowablePropBase::ACPThrowablePropBase()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(StaticMeshComponent);
}

bool ACPThrowablePropBase::CanInteract_Implementation(AActor* Interactor)
{
	return IsValid(Interactor);
}

FName ACPThrowablePropBase::GetInteractionName_Implementation()
{
	return FName(TEXT("Prop"));
}

bool ACPThrowablePropBase::AttachAsHeld(USceneComponent* CarryAnchor)
{
	if (!IsValid(CarryAnchor) || !IsValid(StaticMeshComponent))
	{
		return false;
	}

	// 바닥에서 움직이고 있었다면 기존 속도 제거
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
	SetActorRelativeLocation(FVector::ZeroVector);
	SetActorRelativeRotation(FRotator::ZeroRotator);

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

	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 재료 액터가 기본 물리 시뮬레이션을 사용하고 있으면 true, 아니면 false
	StaticMeshComponent->SetSimulatePhysics(false);
}