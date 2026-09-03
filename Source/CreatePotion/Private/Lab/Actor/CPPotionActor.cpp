// CPPotionActor.cpp

#include "Lab/Actor/CPPotionActor.h"

#include "Components/StaticMeshComponent.h"
#include "Data/CPForageableItemData.h"
#include "GameFramework/Pawn.h"
#include "Items/Potion/CPPotionImpactComponent.h"

ACPPotionActor::ACPPotionActor()
{
    PotionImpactComponent = CreateDefaultSubobject<UCPPotionImpactComponent>(TEXT("PotionImpact"));
}

FName ACPPotionActor::GetInteractionName_Implementation()
{
    return WorkingIngredient.SourceItemData ? 
        FName(*WorkingIngredient.SourceItemData->DisplayName.ToString()) : Super::GetInteractionName_Implementation();   
}

void ACPPotionActor::ApplySpawnImpulse(const FVector& SpawnImpulse)
{
    if (!IsValid(StaticMeshComponent)) return;
    
    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StaticMeshComponent->SetGenerateOverlapEvents(true);
    StaticMeshComponent->SetLinearDamping(PostImpactLinearDamping);
    StaticMeshComponent->SetAngularDamping(PostImpactAngularDamping);
    StaticMeshComponent->SetSimulatePhysics(true);
    StaticMeshComponent->WakeAllRigidBodies();
    StaticMeshComponent->AddImpulse(SpawnImpulse);
    
    SetPropState(ECPThrowablePropState::Dropped);
    StartRestCheck();
}

void ACPPotionActor::HandleThrowStarted(AActor* Thrower)
{
    Super::HandleThrowStarted(Thrower);
    if (!IsValid(PotionImpactComponent)) return;

    APawn* ThrowingPawn = Cast<APawn>(Thrower);
    if (!IsValid(ThrowingPawn)) return;
    
    // Impact 처리
    PotionImpactComponent->EnableImpactProcessing(ThrowingPawn);
}

void ACPPotionActor::HandleThrownImpact(AActor* OtherActor, const FHitResult& HitResult)
{
    Super::HandleThrownImpact(OtherActor, HitResult);

    if (bExplosionTriggered || !IsValid(PotionImpactComponent)) return;
    
    // 플레이어 제외
    if (IsValid(OtherActor) && OtherActor == LastThrower.Get()) return;

    // 포션 컴포넌트에서 첫 Impact시 폭발로 넘어감
    if (!PotionImpactComponent->TryTriggerPotionImpact(HitResult, GetWorkingIngredient().CurrentEffects)) return;

    TriggerPotionExplosion(HitResult);
}

void ACPPotionActor::TriggerPotionExplosion(const FHitResult& HitResult)
{
    if (bExplosionTriggered) return;
    bExplosionTriggered = true;

    // Base의 정지 확인 타이머 중단
    StopRestCheck();

    if (IsValid(PotionImpactComponent))
    {
        PotionImpactComponent->DisableImpactProcessing();
    }

    if (IsValid(StaticMeshComponent))
    {
        StaticMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
        StaticMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        StaticMeshComponent->SetSimulatePhysics(false);
        StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        StaticMeshComponent->SetGenerateOverlapEvents(false);
        StaticMeshComponent->SetVisibility(false, true);
    }

    SetActorEnableCollision(false);

    // 이펙트 생성 지점(나이아가라, 사운드)
    K2_OnPotionExploded(HitResult);
    Destroy();
}