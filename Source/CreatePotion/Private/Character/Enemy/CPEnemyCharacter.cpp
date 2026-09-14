// CPEnemyCharacter.cpp


#include "Character/Enemy/CPEnemyCharacter.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "BrainComponent.h"
#include "Character/CPHealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
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

    HealthComponent = CreateDefaultSubobject<UCPHealthComponent>(TEXT("HealthComponent"));
}

void ACPEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    AbilitySystemComponent->InitAbilityActorInfo(this, this);
    if (HasAuthority())
    {
        GrantPotionReactionAbilities();
    }

    HealthComponent->OnDeath.AddUniqueDynamic(this, &ACPEnemyCharacter::HandleDeath);
}

void ACPEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    HealthComponent->OnDeath.RemoveDynamic(this, &ACPEnemyCharacter::HandleDeath);
    CancelAttack();

    Super::EndPlay(EndPlayReason);
}

UAbilitySystemComponent* ACPEnemyCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
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

    bIsDead = true;
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

    if (DistanceSquared > FMath::Square(AttackRange))
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

    UE_LOG(LogTemp, Log, TEXT("[EnemyAttack] 공격 시작: %s"), *GetName());

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
        UE_LOG(LogTemp, Log, TEXT("[EnemyAttack] 공격 빗나감: %s"), *GetName());
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


