// CPRespawnComponent.cpp

#include "Character/CPRespawnComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#include "Animation/AnimInstance.h"
#include "Character/CPHealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"
#include "Character/CPInteractionComponent.h"

UCPRespawnComponent::UCPRespawnComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCPRespawnComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());

    if (!IsValid(OwnerCharacter))
    {
        return;
    }

    HealthComponent = OwnerCharacter->FindComponentByClass<UCPHealthComponent>();

    if (!IsValid(HealthComponent))
    {
        return;
    }
    
    interactionComponent = OwnerCharacter->FindComponentByClass<UCPInteractionComponent>();
    
    if (!IsValid(RespawnAnimationMesh))
    {
        RespawnAnimationMesh = OwnerCharacter->GetMesh();
    }

    CachedRespawnStart = FindRespawnStart();

    if (!CachedRespawnStart.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Respawn] 부활 PlayerStart를 확인하세요. Tag: %s"), *RespawnStartTag.ToString());
    }

    HealthComponent->OnDeath.AddUniqueDynamic(this, &UCPRespawnComponent::HandleOwnerDeath);
}

void UCPRespawnComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RespawnTimerHandle);
    }

    if (IsValid(HealthComponent))
    {
        HealthComponent->OnDeath.RemoveDynamic(
            this,
            &UCPRespawnComponent::HandleOwnerDeath
        );
    }

    Super::EndPlay(EndPlayReason);
}

void UCPRespawnComponent::SetRespawnAnimationMesh(USkeletalMeshComponent* InMesh)
{
    if (!IsValid(InMesh) || InMesh->GetOwner() != GetOwner())
    {
        return;
    }

    RespawnAnimationMesh = InMesh;
}

void UCPRespawnComponent::HandleOwnerDeath(AActor* DamageCauser)
{
    UWorld* World = GetWorld();

    if (!World || !IsValid(HealthComponent) || !HealthComponent->IsDead())
    {
        return;
    }
    
    if (IsValid(interactionComponent)){
        interactionComponent->SetInteractionBlocked(true);
    }

    if (GameOverSound && !World->GetTimerManager().IsTimerActive(RespawnTimerHandle) && OwnerCharacter->IsPlayerControlled())
    {
        UGameplayStatics::PlaySound2D(this, GameOverSound);
    }

    const float SafeDelay = FMath::IsFinite(RespawnDelay) ? FMath::Max(RespawnDelay, 0.01f) : 5.f;

    // 같은 핸들에 설정하므로 중복 타이머가 생기지 않음
    World->GetTimerManager().SetTimer(
        RespawnTimerHandle,
        this,
        &UCPRespawnComponent::HandleRespawnTimer,
        SafeDelay,
        false
    );
}

void UCPRespawnComponent::HandleRespawnTimer()
{
    Respawn();
}

APlayerStart* UCPRespawnComponent::FindRespawnStart() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    for (TActorIterator<APlayerStart> It(World); It; ++It)
    {
        APlayerStart* Candidate = *It;
        if (!IsValid(Candidate)) continue;
        if (!RespawnStartTag.IsNone() && Candidate->PlayerStartTag != RespawnStartTag) continue;
        return Candidate;
    }
    return nullptr;
}

bool UCPRespawnComponent::Respawn()
{
    UWorld* World = GetWorld();

    if (!World || !IsValid(OwnerCharacter) || !IsValid(HealthComponent))
    {
        return false;
    }

    World->GetTimerManager().ClearTimer(RespawnTimerHandle);

    // 이미 부활했거나 살아 있다면
    if (!HealthComponent->IsDead())
    {
        return false;
    }

    UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();

    if (!IsValid(Movement))
    {
        return false;
    }

    if (!CachedRespawnStart.IsValid())
    {
        CachedRespawnStart = FindRespawnStart();
    }

    APlayerStart* RespawnStart = CachedRespawnStart.Get();

    if (!IsValid(RespawnStart))
    {
        return false;
    }

    const FVector RespawnLocation = RespawnStart->GetActorLocation();

    const FRotator RespawnRotation(0.f, RespawnStart->GetActorRotation().Yaw, 0.f);

    // 부활 시 이동, 힘(Impulse) 제거
    Movement->StopMovementImmediately();
    Movement->ClearAccumulatedForces();

    // 목적지에 들어갈 수 없으면 실패(충돌검사)
    const bool bTeleported = OwnerCharacter->TeleportTo(
        RespawnLocation,
        RespawnRotation,
        false,
        false
    );

    if (!bTeleported)
    {
        // 충돌검사 통과 X
        return false;
    }
    
    if (IsValid(RespawnAnimationMesh))
    {
        if (UAnimInstance* AnimInstance = RespawnAnimationMesh->GetAnimInstance())
        {
            // 부활 시 몽타주 모두 중단
            AnimInstance->Montage_Stop(0.f);
        }
    }

    // 몽타주 종료 후 이동, 힘(Impulse) 제거
    Movement->StopMovementImmediately();
    Movement->ClearAccumulatedForces();

    // 남아 있을 수 있는 입력 제거
    OwnerCharacter->ConsumeMovementInputVector();

    if (bResetControlRotation)
    {
        if (AController* Controller = OwnerCharacter->GetController())
        {
            Controller->SetControlRotation(RespawnRotation);
        }
    }

    Movement->SetMovementMode(MOVE_Walking);

    // 회복 타이머 정리
    HealthComponent->RestoreFullHealth();
    
    if (IsValid(interactionComponent)){
        interactionComponent->SetInteractionBlocked(false);
    }
    
    if (RespawnSound && OwnerCharacter->IsPlayerControlled()) UGameplayStatics::PlaySound2D(this, RespawnSound);
    OnRespawnCompleted.Broadcast();
    
    return true;
}
