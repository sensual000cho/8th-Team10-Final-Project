// CPPotionActor.cpp

#include "Lab/Actor/CPPotionActor.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Items/Potion/CPPotionImpactComponent.h"

ACPPotionActor::ACPPotionActor()
{
    PotionImpactComponent = CreateDefaultSubobject<UCPPotionImpactComponent>(TEXT("PotionImpact"));
}

FText ACPPotionActor::GetInteractionPrompt_Implementation()
{
    return FText::FromString(TEXT("들기"));
}

FName ACPPotionActor::GetInteractionName_Implementation()
{
    return FName(TEXT("수상한 포션"));
}

UCPPotionImpactComponent* ACPPotionActor::GetPotionImpactComponent() const
{
    return PotionImpactComponent;
}

void ACPPotionActor::HandleThrowStarted(AActor* Thrower)
{
    Super::HandleThrowStarted(Thrower);

    if (!IsValid(PotionImpactComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("[PotionActor] PotionImpactComponent가 없습니다."));
        return;
    }

    APawn* ThrowingPawn = Cast<APawn>(Thrower);

    if (!IsValid(ThrowingPawn))
    {
        UE_LOG(LogTemp, Warning, TEXT("[PotionActor] 투척자를 Pawn으로 변환할 수 없습니다."));
        return;
    }

    PotionImpactComponent->SetPotionEffectTags(GetWorkingIngredient().CurrentEffects);
    if (!PotionImpactComponent->EnableImpactProcessing(ThrowingPawn))
    {
        UE_LOG(LogTemp, Warning, TEXT("[PotionActor] Impact 처리 활성화에 실패했습니다."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[PotionActor] Impact 처리 활성화: %s"), *GetName());
}

void ACPPotionActor::HandleThrownImpact(AActor* OtherActor, const FHitResult& HitResult)
{
    Super::HandleThrownImpact(OtherActor, HitResult);

    if (bExplosionTriggered || !IsValid(PotionImpactComponent))
    {
        return;
    }
    
    // 플레이어 제외
    if (IsValid(OtherActor) && OtherActor == GetLastThrower())
    {
        return;
    }

    // 포션 컴포넌트에서 첫 Impact시 폭발로 넘어감
    if (!PotionImpactComponent->TryTriggerPotionImpact(HitResult))
    {
        return;
    }

    TriggerPotionExplosion(HitResult);
}

void ACPPotionActor::TriggerPotionExplosion(const FHitResult& HitResult)
{
    if (bExplosionTriggered)
    {
        return;
    }

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

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[PotionActor] 포션 폭발: %s / 충돌 대상: %s"),
        *GetName(),
        IsValid(HitResult.GetActor()) ? *HitResult.GetActor()->GetName() : TEXT("없음")
    );

    Destroy();
}


