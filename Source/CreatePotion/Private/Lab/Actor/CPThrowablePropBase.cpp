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