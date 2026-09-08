#include "Field/Actor/CPBaseFieldActor.h"

ACPBaseFieldActor::ACPBaseFieldActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACPBaseFieldActor::BeginPlay()
{
	Super::BeginPlay();
	
}