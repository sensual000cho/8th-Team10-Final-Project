#include "NPC/CPBaseNPC.h"
#include "Data/CPNPCDataAsset.h"
#include "Components/CapsuleComponent.h"
#include "Components/StateTreeComponent.h"
#include "Animation/AnimSequence.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/QuestManager.h"

ACPBaseNPC::ACPBaseNPC()
{
	PrimaryActorTick.bCanEverTick = false;
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
        GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
    }
    //StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));

}

void ACPBaseNPC::OnInteract_Implementation(AActor* Interactor)
{
}

FText ACPBaseNPC::GetInteractionPrompt_Implementation()
{
    if (!CanInteract_Implementation(nullptr))
    {
        return FText::GetEmpty();
    }

    UE_LOG(LogTemp, Log, TEXT("[대화하기] GetInteractionPrompt called"));

    //확인용, 추후 삭제 예정
    if (NPCData && GetGameInstance())
    {
        if (UQuestManager* QuestManager = GetGameInstance()->GetSubsystem<UQuestManager>())
        {
            for (const FName& QuestID : NPCData->TownQuestIDs)
            {
                if (QuestID.IsNone()) { continue; }

                EQuestState CurrentState = QuestManager->GetQuestState(QuestID);
                if (CurrentState == EQuestState::NotAccepted)
                {
                    FText FullScript = QuestManager->GetQuestFullText(QuestID);  
                    UE_LOG(LogTemp, Log, TEXT("[퀘스트 대사 확인] QuestID: %s"), *QuestID.ToString());
                    UE_LOG(LogTemp, Log, TEXT("[대사 내용]: %s"), *FullScript.ToString());
                 
                }
            }
            for (const FName& QuestID : NPCData->LabQuestIDs)
            {
                if (QuestID.IsNone()) { continue; }

                if (QuestManager->GetQuestState(QuestID) == EQuestState::Accepted)
                {
                    FText HintScript = QuestManager->GetSessionHintText(QuestID);
                    UE_LOG(LogTemp, Log, TEXT("[리퀘스트 대사 확인] QuestID: %s"), *QuestID.ToString());
                    UE_LOG(LogTemp, Log, TEXT("[대사 내용]: %s"), *HintScript.ToString());
                  
                }
            }
        }
    }
    return FText::FromString(TEXT("F : 대화하기"));
}

bool ACPBaseNPC::CanInteract_Implementation(AActor* Interactor)
{
    UE_LOG(LogTemp, Log, TEXT("[%s] CanInteract called by %s"),
        *GetName(),
        Interactor ? *Interactor->GetName() : TEXT("Unknown"));
    return true;
}

bool ACPBaseNPC::GetDialogueEntryForCurrentState(FCPNPCDialogueEntry& OutEntry) const
{
    if (!NPCData) { return false; }

    if (const FCPNPCSituationData* SituationData = NPCData->SituationData.Find(CurrentSituation))
    {
        if (const FCPNPCDialogueEntry* Entry = SituationData->EmotionVariants.Find(CurrentEmotion))
        {
            OutEntry = *Entry;
            return true;
        }
        else if (const FCPNPCDialogueEntry* DefaultEntry = SituationData->EmotionVariants.Find(ECPNPCEmotion::Neutral))
        {
            OutEntry = *DefaultEntry;
            return true;
        }
    }
    return false;
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
    if (!NPCData) { return; }
    
    USkeletalMesh* LoadedMesh = NPCData->NPCMesh.LoadSynchronous();
    if (LoadedMesh)
    {
        GetMesh()->SetSkeletalMesh(LoadedMesh);
        FitCapsuleToMesh(LoadedMesh);
    }

    UAnimSequence* LoadedAnim = NPCData->IdleAnimation.LoadSynchronous();
    if (LoadedAnim)
    {
        GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        GetMesh()->PlayAnimation(LoadedAnim, true); // 반복 재생
    }

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


