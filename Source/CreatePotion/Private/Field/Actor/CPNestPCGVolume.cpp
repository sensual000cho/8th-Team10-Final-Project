#include "Field/Actor/CPNestPCGVolume.h"
#include "PCGComponent.h"
#include "PCGGraph.h"

void ACPNestPCGVolume::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimerForNextTick(
		this,
		&ThisClass::GenerateNests
	);
}

void ACPNestPCGVolume::GenerateNests()
{
	if (!IsValid(PCGComponent)) return;

	UPCGGraphInstance* GraphInstance = PCGComponent->GetGraphInstance();
	if (!IsValid(GraphInstance)) return;
	
	const int32 NestSeed = FMath::Rand();
	
	GraphInstance->SetGraphParameter<int32>(TEXT("NestSeed"), NestSeed);
	
	PCGComponent->Generate(true);
}
