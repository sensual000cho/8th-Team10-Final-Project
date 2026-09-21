// CPEnemyCharacter.cpp


#include "Character/Enemy/CPEnemyCharacter.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "BrainComponent.h"
#include "Character/CPHealthComponent.h"
#include "Character/Enemy/CPEnemyAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

ACPEnemyCharacter::ACPEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);

    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);

    EnemyAttributeSet = CreateDefaultSubobject<UCPEnemyAttributeSet>(TEXT("EnemyAttributeSet"));

    DeathPhysicsSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DeathPhysicsSphere"));
    DeathPhysicsSphere->SetupAttachment(GetCapsuleComponent());
    DeathPhysicsSphere->InitSphereRadius(GetCapsuleComponent()->GetUnscaledCapsuleRadius());
    DeathPhysicsSphere->SetCollisionProfileName(TEXT("PhysicsActor"));
    DeathPhysicsSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DeathPhysicsSphere->SetEnableGravity(false);
}

void ACPEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    AbilitySystemComponent->InitAbilityActorInfo(this, this);
    InitialMeshRelativeRotation = GetMesh()->GetRelativeRotation();
    if (HasAuthority())
    {
        GrantPotionReactionAbilities();
    }

    HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCPEnemyAttributeSet::GetHealthAttribute()).AddUObject(this, &ACPEnemyCharacter::HandleHealthChanged);
}

void ACPEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    AbilitySystemComponent->CancelAllAbilities();
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCPEnemyAttributeSet::GetHealthAttribute()).Remove(HealthChangedDelegateHandle);
    CancelAttack();

    Super::EndPlay(EndPlayReason);
}

UAbilitySystemComponent* ACPEnemyCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ACPEnemyCharacter::Landed(const FHitResult& Hit)
{
    LandingVelocity = GetVelocity();
    LandingSurfaceNormal = Hit.ImpactNormal;
    bHandlingLanding = true;
    Super::Landed(Hit);
    bHandlingLanding = false;
}

float ACPEnemyCharacter::GetHealth() const
{
    return EnemyAttributeSet->GetHealth();
}

void ACPEnemyCharacter::InitializeSlimeSplit(const ACPEnemyCharacter& SourceEnemy, float ScaleMultiplier)
{
    const float SourceHealth = SourceEnemy.GetHealth();
    const bool bIsClone = this != &SourceEnemy;
    const float PreviousHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    // 복제본은 원본의 일시적인 Giant 배율을 제외한 분열 크기로 시작한다.
    const float SourceGiantScale = bIsClone ? SourceEnemy.GiantScaleMultiplier : 1.0f;
    const FVector SplitScale = SourceEnemy.GetActorScale3D() * ScaleMultiplier / SourceGiantScale;
    SlimeSplitCount = SourceEnemy.SlimeSplitCount + 1;
    // 중심 간 3D 거리 판정이므로 분열해도 공격 사거리는 원본 값을 유지한다.
    AttackRange = SourceEnemy.AttackRange;
    GetCharacterMovement()->MaxStepHeight = SourceEnemy.GetCharacterMovement()->MaxStepHeight * ScaleMultiplier / SourceGiantScale;

    // Capsule, Mesh, 부착된 사망 물리 구체를 함께 줄이고 경로 탐색 크기도 갱신한다.
    SetActorScale3D(SplitScale);
    if (bIsClone && SourceGiantScale != 1.0f)
    {
        AddActorWorldOffset(FVector::UpVector * (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - PreviousHalfHeight), false);
    }
    UpdateNavAgent();
    if (bIsClone)
    {
        AbilitySystemComponent->SetNumericAttributeBase(UCPEnemyAttributeSet::GetHealthAttribute(), SourceHealth);
    }
}

void ACPEnemyCharacter::SetGiantMultipliers(float ScaleMultiplier, float SpeedMultiplier, float ImpulseMultiplier)
{
    const float PreviousHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const float ScaleRatio = ScaleMultiplier / GiantScaleMultiplier;
    SetActorScale3D(GetActorScale3D() * ScaleRatio);
    GetCharacterMovement()->MaxStepHeight *= ScaleRatio;
    GetCharacterMovement()->MaxWalkSpeed *= SpeedMultiplier / GiantSpeedMultiplier;
    GiantScaleMultiplier = ScaleMultiplier;
    GiantSpeedMultiplier = SpeedMultiplier;
    AttackImpulseMultiplier = ImpulseMultiplier;
    UpdateNavAgent();

    // 발밑 높이를 유지하고 엔진의 기본 Teleport 충돌 보정만 사용한다.
    const FVector AdjustedLocation = GetActorLocation() + FVector::UpVector * (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - PreviousHalfHeight);
    TeleportTo(AdjustedLocation, GetActorRotation(), false, false);
    GetCharacterMovement()->bForceNextFloorCheck = true;
}

float ACPEnemyCharacter::ApplyDamage(float DamageAmount, AActor* DamageCauser)
{
    if (DamageAmount <= 0.0f || bIsDead)
    {
        return 0.0f;
    }

    const float PreviousHealth = EnemyAttributeSet->GetHealth();
    PendingDamageCauser = DamageCauser;
    AbilitySystemComponent->SetNumericAttributeBase(UCPEnemyAttributeSet::GetHealthAttribute(), PreviousHealth - DamageAmount);
    PendingDamageCauser.Reset();

    return PreviousHealth - EnemyAttributeSet->GetHealth();
}

void ACPEnemyCharacter::HandleHealthChanged(const FOnAttributeChangeData& ChangeData)
{
    if (ChangeData.NewValue <= 0.0f)
    {
        HandleDeath(PendingDamageCauser.Get());
    }
}

void ACPEnemyCharacter::GrantPotionReactionAbilities()
{
    for (const TSubclassOf<UGameplayAbility>& AbilityClass : PotionReactionAbilities)
    {
        if (AbilityClass && !AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass))
        {
            AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
        }
    }
}

void ACPEnemyCharacter::HandleDeath(AActor* DamageCauser)
{
    if (bIsDead)
    {
        return;
    }

    const FVector DeathVelocity = bHandlingLanding ? LandingVelocity : GetVelocity();
    const FVector DeathSurfaceNormal = bHandlingLanding ? LandingSurfaceNormal : FVector::UpVector;

    bIsDead = true;
    // 물리 구체와 Mesh가 분리되기 전에 Giant를 해제한다.
    OnEnemyDying.Broadcast();
    CancelAttack();
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    if (AAIController* AIController = Cast<AAIController>(GetController()))
    {
        AIController->StopMovement();
        if (UBrainComponent* BrainComponent = AIController->GetBrainComponent())
        {
            BrainComponent->StopLogic(TEXT("Enemy dead"));
        }
    }

    PlayDeathPose();
    StartDeathPhysics(DeathVelocity, DeathSurfaceNormal);
    SetLifeSpan(10.0f);
}

void ACPEnemyCharacter::PlayDeathPose()
{
    USkeletalMeshComponent* MeshComponent = GetMesh();
    if (!IsValid(MeshComponent))
    {
        return;
    }

    if (IsValid(DeathPoseAnimation))
    {
        MeshComponent->PlayAnimation(DeathPoseAnimation, false);
    }
}

void ACPEnemyCharacter::StartDeathPhysics(const FVector& DeathVelocity, const FVector& SurfaceNormal)
{
    UCapsuleComponent* CharacterCapsule = GetCapsuleComponent();
    USkeletalMeshComponent* MeshComponent = GetMesh();
    const FQuat DeathSphereRotation = MeshComponent->GetComponentQuat() * InitialMeshRelativeRotation.Quaternion().Inverse();

    DeathPhysicsSphere->SetWorldLocation(MeshComponent->Bounds.Origin, false, nullptr, ETeleportType::TeleportPhysics);
    DeathPhysicsSphere->SetWorldRotation(DeathSphereRotation, false, nullptr, ETeleportType::TeleportPhysics);
    DeathPhysicsSphere->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    MeshComponent->AttachToComponent(DeathPhysicsSphere, FAttachmentTransformRules::KeepWorldTransform);
    MeshComponent->SetRelativeRotation(InitialMeshRelativeRotation, false, nullptr, ETeleportType::TeleportPhysics);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CharacterCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DeathPhysicsSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DeathPhysicsSphere->SetEnableGravity(true);
    DeathPhysicsSphere->SetLinearDamping(1.5f);
    DeathPhysicsSphere->SetAngularDamping(3.0f);
    DeathPhysicsSphere->SetSimulatePhysics(true);
    DeathPhysicsSphere->SetPhysicsLinearVelocity(DeathVelocity);

    const FVector RollVelocity = FVector::VectorPlaneProject(DeathVelocity, SurfaceNormal);
    const FVector RollAngularVelocity = FVector::CrossProduct(SurfaceNormal, RollVelocity) / DeathPhysicsSphere->GetScaledSphereRadius();
    DeathPhysicsSphere->SetPhysicsAngularVelocityInRadians(RollAngularVelocity);
}

bool ACPEnemyCharacter::IsTargetInAttackReach(AActor* Target) const
{
    if (!IsValid(Target) || Target == this || !GetWorld())
    {
        return false;
    }

    const UCPHealthComponent* TargetHealthComponent = Target->FindComponentByClass<UCPHealthComponent>();

    if (!IsValid(TargetHealthComponent) || TargetHealthComponent->IsDead())
    {
        return false;
    }

    // 높이가 다른 대상까지 공격하지 않도록 3D 거리 사용
    const float DistanceSquared = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());

    if (DistanceSquared > FMath::Square(GetEffectiveAttackRange()))
    {
        return false;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    FHitResult Hit;

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        GetActorLocation(),
        Target->GetActorLocation(),
        ECC_Visibility,
        QueryParams
    );

    return !bHit || Hit.GetActor() == Target;
}

bool ACPEnemyCharacter::CanAttackTarget(AActor* Target) const
{
    if (bIsDead || bIsAttacking || AttackMontages.IsEmpty() || !GetWorld())
    {
        return false;
    }

    if (GetWorld()->GetTimeSeconds() < NextAttackAllowedTime)
    {
        return false;
    }

    return IsTargetInAttackReach(Target);
}

bool ACPEnemyCharacter::TryStartAttack(AActor* Target)
{
    if (!CanAttackTarget(Target))
    {
        return false;
    }

    UAnimInstance* AnimInstance = IsValid(GetMesh()) ? GetMesh()->GetAnimInstance() : nullptr;

    if (!IsValid(AnimInstance))
    {
        return false;
    }

    // 기존 Chase 중단
    if (AAIController* AIController = Cast<AAIController>(GetController()))
    {
        AIController->StopMovement();
    }

    GetCharacterMovement()->StopMovementImmediately();

    // 공격 시작 시 타깃 방향으로 Yaw만 회전
    const FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();

    if (!ToTarget.IsNearlyZero())
    {
        SetActorRotation(FRotator(0.f, ToTarget.Rotation().Yaw, 0.f));
    }

    RandomMontageNum = FMath::RandRange(0, AttackMontages.Num() - 1);
    
    if (!IsValid(AttackMontages[RandomMontageNum]))
    {
        return false;
    }
    
    const float PlayedLength = AnimInstance->Montage_Play(AttackMontages[RandomMontageNum]);

    if (PlayedLength <= 0.f)
    {
        return false;
    }

    AttackTarget = Target;
    bIsAttacking = true;
    bAttackHitConsumed = false;
    bAttackHitEnabled = true;
    bLastAttackCompletedNormally = false;

    // 블렌드아웃이 시작되면 Hit Notify를 차단
    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &ACPEnemyCharacter::HandleAttackBlendingOut);

    AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, AttackMontages[RandomMontageNum]);

    // 정상 종료와 중단을 구분
    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &ACPEnemyCharacter::HandleAttackEnded);

    AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontages[RandomMontageNum]);

    // UE_LOG(LogTemp, Log, TEXT("[EnemyAttack] 공격 시작: %s"), *GetName());

    return true;
}

void ACPEnemyCharacter::ApplyAttackHit()
{
    if (!bIsAttacking || !bAttackHitEnabled || bAttackHitConsumed)
    {
        return;
    }

    // 빗나가더라도 이번 공격의 타격 기회는 소비한다.
    bAttackHitConsumed = true;

    AActor* Target = AttackTarget.Get();

    // 공격을 시작한 뒤 도망갔거나 벽(나무 등)에 가려졌는지 다시 검사
    if (!IsTargetInAttackReach(Target))
    {
        // UE_LOG(LogTemp, Log, TEXT("[EnemyAttack] 공격 빗나감: %s"), *GetName());
        return;
    }

    const FVector DirectionToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    const float Dot = FVector::DotProduct(GetActorForwardVector().GetSafeNormal2D(),DirectionToTarget);
    const float CosThreshold = FMath::Cos(FMath::DegreesToRadians(AttackHalfAngle));

    // 타격 순간 뒤쪽으로 피한 대상에게는 피해를 주지 않음
    if (!DirectionToTarget.IsNearlyZero() && Dot < CosThreshold)
    {
        return;
    }

    UCPHealthComponent* TargetHealthComponent = Target->FindComponentByClass<UCPHealthComponent>();

    if (IsValid(TargetHealthComponent))
    {
        // 피해 처리 중 이벤트가 발생할 수 있으므로 마지막에 호출
        TargetHealthComponent->ApplyDamage(AttackDamage, this);
    }
}

void ACPEnemyCharacter::HandleAttackBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    if (!bIsAttacking || !AttackMontages.IsValidIndex(RandomMontageNum) || Montage != AttackMontages[RandomMontageNum].Get())
    {
        return;
    }

    bAttackHitEnabled = false;
}

void ACPEnemyCharacter::HandleAttackEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!bIsAttacking || !AttackMontages.IsValidIndex(RandomMontageNum) || Montage != AttackMontages[RandomMontageNum].Get())
    {
        return;
    }

    bIsAttacking = false;
    bAttackHitEnabled = false;
    bLastAttackCompletedNormally = !bInterrupted;
    AttackTarget.Reset();

    if (GetWorld())
    {
        NextAttackAllowedTime = GetWorld()->GetTimeSeconds() + AttackCooldown;
    }
}

void ACPEnemyCharacter::CancelAttack()
{
    if (!bIsAttacking)
    {
        return;
    }

    // Stop 호출 전에 타격과 공격 상태부터 정리
    bIsAttacking = false;
    bAttackHitEnabled = false;
    bLastAttackCompletedNormally = false;
    AttackTarget.Reset();

    if (GetWorld())
    {
        NextAttackAllowedTime = GetWorld()->GetTimeSeconds() + AttackCooldown;
    }

    if (UAnimInstance* AnimInstance = IsValid(GetMesh()) ? GetMesh()->GetAnimInstance() : nullptr)
    {
        AnimInstance->Montage_Stop(0.15f, AttackMontages[RandomMontageNum]);
    }
}

void ACPEnemyCharacter::RequestVictory()
{
    bVictoryRequested = true;
}

void ACPEnemyCharacter::ClearVictoryRequest()
{
    bVictoryRequested = false;
}

bool ACPEnemyCharacter::StartVictoryRequest()
{
    if (bIsVictoryPlaying)
    {
        return false;
    }

    bVictoryCompletedNormally = false;
    ActiveVictoryMontage = nullptr;
    
    if (VictoryMontages.IsEmpty())
    {
        return false;
    }

    UAnimInstance* AnimInstance = IsValid(GetMesh()) ? GetMesh()->GetAnimInstance() : nullptr;

    if (!IsValid(AnimInstance))
    {
        return false;
    }

    const int32 MontageIndex = FMath::RandRange(0, VictoryMontages.Num() - 1);
    UAnimMontage* SelectedMontage = VictoryMontages[MontageIndex];
    
    if (!IsValid(SelectedMontage))
    {
        return false;
    }
    
    CancelAttack();

    if (AAIController* AIController = Cast<AAIController>(GetController()))
    {
        AIController->StopMovement();
    }

    GetCharacterMovement()->StopMovementImmediately();
    const float PlayedLength = AnimInstance->Montage_Play(SelectedMontage);

    if (PlayedLength <= 0.f)
    {
        return false;
    }
    
    ActiveVictoryMontage = SelectedMontage;
    bIsVictoryPlaying = true;

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &ACPEnemyCharacter::HandleVictoryEnded);

    AnimInstance->Montage_SetEndDelegate(EndDelegate, SelectedMontage);

    return true;
}

void ACPEnemyCharacter::HandleVictoryEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != ActiveVictoryMontage.Get())
    {
        return;
    }
    
    bIsVictoryPlaying = false;
    bVictoryCompletedNormally = !bInterrupted;
    ActiveVictoryMontage = nullptr;
}
