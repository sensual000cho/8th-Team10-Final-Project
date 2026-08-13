// CPLabContainerActor.cpp

#include "Lab/Actor/CPLabContainerActor.h"

#include "Components/CPLabContainerComponent.h"

ACPLabContainerActor::ACPLabContainerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	LabContainerComponent = CreateDefaultSubobject<UCPLabContainerComponent>(TEXT("LabContainerComponent"));
}

