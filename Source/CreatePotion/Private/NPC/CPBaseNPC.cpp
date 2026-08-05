#include "NPC/CPBaseNPC.h"
#include "Data/CPNPCDataAsset.h"
#include "Components/CapsuleComponent.h"

ACPBaseNPC::ACPBaseNPC()
{
	PrimaryActorTick.bCanEverTick = false;

}

void ACPBaseNPC::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	InitializeFromDataAsset();
}

void ACPBaseNPC::BeginPlay()
{
	Super::BeginPlay();
	InitializeFromDataAsset();
	
}

void ACPBaseNPC::InitializeFromDataAsset()
{
    if (!NPCData)
    {
        return;
    }
    
    USkeletalMesh* LoadedMesh = NPCData->NPCMesh.LoadSynchronous();
    if (!LoadedMesh)
    {
        return;
    }

    GetMesh()->SetSkeletalMesh(LoadedMesh);

    FitCapsuleToMesh(LoadedMesh);

    if (NPCData->NPCName != NAME_None)
    {
        Tags.AddUnique(NPCData->NPCName);
    }

    SetActorTransform(NPCData->SpawnTransform);
}

void ACPBaseNPC::FitCapsuleToMesh(USkeletalMesh* InMesh)
{
    if (!InMesh)
    {
        return;
    }

    const FBoxSphereBounds MeshBounds = InMesh->GetBounds();
    const float MeshHalfHeight = MeshBounds.BoxExtent.Z;
    const float MeshRadius = MeshHalfHeight * (NPCData->CapsuleRadiusRatio);

    GetCapsuleComponent()->SetCapsuleSize(MeshRadius, MeshHalfHeight);
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -MeshHalfHeight));
}