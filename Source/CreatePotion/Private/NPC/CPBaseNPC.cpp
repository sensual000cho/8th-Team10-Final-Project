#include "NPC/CPBaseNPC.h"
#include "NPC/CPNPCDataAsset.h"
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

void ACPBaseNPC::OnInteract_Implementation(AActor* Interactor)
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] OnInteract called by %s"),
        *GetName(),
        Interactor ? *Interactor->GetName() : TEXT("Unknown"));
}

FText ACPBaseNPC::GetInteractionPrompt_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("[%s] GetInteractionPrompt called"), *GetName());
    return FText::FromString(TEXT("대화하기"));
}

bool ACPBaseNPC::CanInteract_Implementation(AActor* Interactor)
{
    UE_LOG(LogTemp, Log, TEXT("[%s] CanInteract called by %s"),
        *GetName(),
        Interactor ? *Interactor->GetName() : TEXT("Unknown"));
    return true;
}