// CPThrowablePropBase.cpp

#include "Lab/Actor/CPThrowablePropBase.h"

#include "AbilitySystemComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

ACPThrowablePropBase::ACPThrowablePropBase()
{
    PrimaryActorTick.bCanEverTick = false;

    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(StaticMeshComponent);
}

void ACPThrowablePropBase::BeginPlay()
{
    Super::BeginPlay();

    if (!IsValid(StaticMeshComponent))
    {
        return;
    }

    // 물리 충돌 시 OnComponentHit이 발생하도록 설정
    StaticMeshComponent->SetNotifyRigidBodyCollision(true);
    StaticMeshComponent->OnComponentHit.AddUniqueDynamic(this, &ACPThrowablePropBase::HandleMeshHit);
}

void ACPThrowablePropBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopRestCheck();

    if (IsValid(StaticMeshComponent))
    {
        StaticMeshComponent->OnComponentHit.RemoveDynamic(this, &ACPThrowablePropBase::HandleMeshHit);
    }

    Super::EndPlay(EndPlayReason);
}

void ACPThrowablePropBase::OnInteract_Implementation(AActor* Interactor)
{
    if (!IsValid(Interactor) || !CanBePickedUp()) return;
    
    // Ability System 탐색
    UAbilitySystemComponent* AbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Interactor);
    if (!IsValid(AbilitySystem)) return;
    
    // 들기, 던지기 GAS 연결
    static const FGameplayTag PickupEventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Carry.Pickup")));
    FGameplayEventData EventData;
    EventData.EventTag = PickupEventTag;
    EventData.Instigator = Interactor;
    EventData.Target = this;
    
    AbilitySystem->HandleGameplayEvent(PickupEventTag, &EventData);
}

bool ACPThrowablePropBase::CanInteract_Implementation(AActor* Interactor)
{
    return IsValid(Interactor) && CanBePickedUp();
}

FName ACPThrowablePropBase::GetInteractionName_Implementation()
{
    return FName(TEXT("Prop"));
}

bool ACPThrowablePropBase::AttachAsHeld(USceneComponent* CarryAnchor)
{
    if (!IsValid(CarryAnchor) || !IsValid(StaticMeshComponent))
    {
        return false;
    }

    // 이미 부착된 경우 성공처리
    if (PropState == ECPThrowablePropState::Held)
    {
        return GetAttachParentActor() == CarryAnchor->GetOwner();
    }

    // 날아가는 중인 Prop은 바로 집을 수는 없음
    if (!CanBePickedUp())
    {
        return false;
    }

    StopRestCheck();

    // 이전 물리 운동 제거
    StaticMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
    StaticMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    StaticMeshComponent->SetSimulatePhysics(false);
    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    const bool bAttached = AttachToComponent(CarryAnchor, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

    if (!bAttached)
    {
        StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        return false;
    }

    SetActorHiddenInGame(false);
    SetActorRelativeLocation(HeldRelativeTransform.GetLocation());
    SetActorRelativeRotation(HeldRelativeTransform.Rotator());

    LastThrower = nullptr;
    bHasHitSinceThrow = false;
    LowSpeedElapsedTime = 0.f;

    SetPropState(ECPThrowablePropState::Held);

    return true;
}

void ACPThrowablePropBase::DetachAsHeld(const FVector& DropLocation)
{
    if (!IsValid(StaticMeshComponent))
    {
        return;
    }

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    SetActorLocation(DropLocation, false, nullptr, ETeleportType::TeleportPhysics);

    SetActorHiddenInGame(false);

    StaticMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
    StaticMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    StaticMeshComponent->SetSimulatePhysics(false);
    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StaticMeshComponent->SetGenerateOverlapEvents(true);

    LastThrower = nullptr;
    bHasHitSinceThrow = false;
    LowSpeedElapsedTime = 0.f;

    StopRestCheck();
    SetPropState(ECPThrowablePropState::Resting);
}

bool ACPThrowablePropBase::GetPropCollisionBounds(FVector& OutOrigin, FVector& OutExtent) const
{
    if (!IsValid(StaticMeshComponent))
    {
        OutOrigin = FVector::ZeroVector;
        OutExtent = FVector::ZeroVector;
        return false;
    }

    OutOrigin = StaticMeshComponent->Bounds.Origin;
    OutExtent = StaticMeshComponent->Bounds.BoxExtent;

    return true;
}

bool ACPThrowablePropBase::Drop(const FVector& DropLocation)
{
    if (!IsValid(StaticMeshComponent) || PropState != ECPThrowablePropState::Held)
    {
        return false;
    }

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    SetActorLocation(DropLocation, false, nullptr, ETeleportType::TeleportPhysics);
    SetActorHiddenInGame(false);

    LastThrower = nullptr;
    bHasHitSinceThrow = false;
    LowSpeedElapsedTime = 0.f;

    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StaticMeshComponent->SetGenerateOverlapEvents(true);
    StaticMeshComponent->SetLinearDamping(PostImpactLinearDamping);
    StaticMeshComponent->SetAngularDamping(PostImpactAngularDamping);
    StaticMeshComponent->SetSimulatePhysics(true);
    StaticMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
    StaticMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    StaticMeshComponent->WakeAllRigidBodies();
    
    SetPropState(ECPThrowablePropState::Dropped);
    StartRestCheck();

    return true;
}

bool ACPThrowablePropBase::Throw(const FVector& Direction, float Speed)
{
    if (!IsValid(StaticMeshComponent) || Direction.IsNearlyZero() || Speed <= 0.f)
    {
        return false;
    }

    if (PropState != ECPThrowablePropState::Held)
    {
        return false;
    }

    const FVector ThrowDirection = Direction.GetSafeNormal();

    // 투척자 저장(현 프로젝트 상 무조건 Player)
    LastThrower = GetAttachParentActor();

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // 물리를 활성화하기 전에 투척 방향으로 이동.
    SetActorLocation(
        GetActorLocation() + ThrowDirection * ThrowStartOffset, 
        false, 
        nullptr, 
        ETeleportType::TeleportPhysics
        );

    SetActorHiddenInGame(false);

    bHasHitSinceThrow = false;
    LowSpeedElapsedTime = 0.f;

    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StaticMeshComponent->SetGenerateOverlapEvents(true);
    
    StaticMeshComponent->SetLinearDamping(FlightLinearDamping);
    StaticMeshComponent->SetAngularDamping(FlightAngularDamping);
    
    StaticMeshComponent->SetSimulatePhysics(true);
    StaticMeshComponent->WakeAllRigidBodies();
    StaticMeshComponent->SetPhysicsLinearVelocity(ThrowDirection * Speed);

    SetPropState(ECPThrowablePropState::Thrown);
    StartRestCheck();

    // 물리 투척 준비가 끝난 후 이벤트를 전달.
    OnPropThrown.Broadcast(LastThrower);
    HandleThrowStarted(LastThrower);

    return true;
}

bool ACPThrowablePropBase::CanBePickedUp() const
{
    if (PropState == ECPThrowablePropState::Resting)
    {
        return true;
    }

    const bool bWasPhysicallyReleased = PropState == ECPThrowablePropState::Thrown || PropState == ECPThrowablePropState::Dropped;

    return bWasPhysicallyReleased && bHasHitSinceThrow;
}

ECPThrowablePropState ACPThrowablePropBase::GetPropState() const
{
    return PropState;
}

bool ACPThrowablePropBase::IsHeld() const
{
    return PropState == ECPThrowablePropState::Held;
}

bool ACPThrowablePropBase::IsThrown() const
{
    return PropState == ECPThrowablePropState::Thrown;
}

bool ACPThrowablePropBase::IsDropped() const
{
    return PropState == ECPThrowablePropState::Dropped;
}

bool ACPThrowablePropBase::IsResting() const
{
    return PropState == ECPThrowablePropState::Resting;
}

AActor* ACPThrowablePropBase::GetLastThrower() const
{
    return IsValid(LastThrower) ? LastThrower.Get() : nullptr;
}

void ACPThrowablePropBase::HandleThrowStarted(AActor* Thrower)
{
    // 자식 Actor에서 필요한 경우 재정의(Potion)
}

void ACPThrowablePropBase::HandleThrownImpact(AActor* OtherActor, const FHitResult& HitResult)
{
    // 자식 Actor에서 필요한 경우 재정의(Potion)
}

void ACPThrowablePropBase::SetPropState(ECPThrowablePropState NewState)
{
    if (PropState == NewState)
    {
        return;
    }

    PropState = NewState;
    OnPropStateChanged.Broadcast(PropState);
}

void ACPThrowablePropBase::StartRestCheck()
{
    if (!GetWorld() || RestCheckInterval <= 0.f)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(RestCheckTimerHandle);
    GetWorldTimerManager().SetTimer(RestCheckTimerHandle, this, &ACPThrowablePropBase::CheckForRest, RestCheckInterval, true);
}

void ACPThrowablePropBase::StopRestCheck()
{
    if (!GetWorld())
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(RestCheckTimerHandle);
}

void ACPThrowablePropBase::CheckForRest()
{
    const bool bCanBecomeResting = PropState == ECPThrowablePropState::Thrown || PropState == ECPThrowablePropState::Dropped;

    if (!bCanBecomeResting || !IsValid(StaticMeshComponent))
    {
        StopRestCheck();
        return;
    }

    // 충돌 이후 느려지는 경우만 검사(공중에서 잠시 느려질 수도 있어서)
    if (!bHasHitSinceThrow)
    {
        LowSpeedElapsedTime = 0.f;
        return;
    }

    const float LinearSpeedSquared = StaticMeshComponent->GetPhysicsLinearVelocity().SizeSquared();
    const float AngularSpeedSquared = StaticMeshComponent->GetPhysicsAngularVelocityInDegrees().SizeSquared();

    const bool bLinearVelocityIsLow = LinearSpeedSquared <= FMath::Square(RestLinearSpeedThreshold);
    const bool bAngularVelocityIsLow = AngularSpeedSquared <= FMath::Square(RestAngularSpeedThreshold);

    if (!bLinearVelocityIsLow || !bAngularVelocityIsLow)
    {
        LowSpeedElapsedTime = 0.f;
        return;
    }

    LowSpeedElapsedTime += RestCheckInterval;

    if (LowSpeedElapsedTime < RestConfirmationDuration)
    {
        return;
    }

    StopRestCheck();
    
    // 느려지면 재워버림
    StaticMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
    StaticMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    StaticMeshComponent->PutAllRigidBodiesToSleep();
    
    SetPropState(ECPThrowablePropState::Resting);

    OnPropRested.Broadcast();
}

void ACPThrowablePropBase::HandleMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& HitResult)
{
    const bool bIsThrown = PropState == ECPThrowablePropState::Thrown;
    const bool bIsDropped = PropState == ECPThrowablePropState::Dropped;
    const bool bIsResting = PropState == ECPThrowablePropState::Resting;
    const bool bIsPhysicsRelease = bIsThrown || bIsDropped;
    
    if (!bIsPhysicsRelease && !bIsResting)
    {
        return;
    }
    
    // 충돌 최초 1회 발생 이후 댐핑 적용
    if (bIsPhysicsRelease)
    {
        const bool bIsFirstHit = !bHasHitSinceThrow;
        bHasHitSinceThrow = true;

        if (bIsFirstHit && IsValid(StaticMeshComponent))
        {
            StaticMeshComponent->SetLinearDamping(PostImpactLinearDamping);
            StaticMeshComponent->SetAngularDamping(PostImpactAngularDamping);
        }
    }
    
    // 바닥에 뒹구는 Prop 속도 조절
    if (CanBePickedUp() && IsValid(OtherActor) && OtherActor->IsA<APawn>() && IsValid(StaticMeshComponent))
    {
        const FVector LinearVelocity = StaticMeshComponent->GetPhysicsLinearVelocity();
        const FVector AngularVelocity = StaticMeshComponent->GetPhysicsAngularVelocityInDegrees();

        StaticMeshComponent->SetPhysicsLinearVelocity(LinearVelocity.GetClampedToMaxSize(PushMaxLinearSpeed));
        StaticMeshComponent->SetPhysicsAngularVelocityInDegrees(AngularVelocity.GetClampedToMaxSize(PushMaxAngularSpeed));
    }
    
    // Dropped인 경우 포션 폭발(Impact) 실행을 하지 않고 return
    // 교체 시 바닥에 Drop하므로 Resting 조건도 추가(Drop하는 순간 Resting 가능성)
    if (bIsResting || bIsDropped)
    {
        return;
    }

    // Base에서는 Hit을 차단하지 않음.
    // PotionImpactComponent 내부의 bImpactTriggered를 이용해 첫 Impact만 처리.
    OnPropHit.Broadcast(OtherActor, HitResult);

    // 반드시 함수의 마지막에서 호출.
    // PotionActor가 이 함수 안에서 Destroy될 위험이 있음. 
    HandleThrownImpact(OtherActor, HitResult);
}