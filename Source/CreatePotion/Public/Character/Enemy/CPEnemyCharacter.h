// CPEnemyCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "CPEnemyCharacter.generated.h"

class UAnimMontage;
class UAnimSequence;
class UAbilitySystemComponent;
class UCPEnemyAttributeSet;
class UGameplayAbility;
class USphereComponent;
struct FOnAttributeChangeData;

UCLASS()
class CREATEPOTION_API ACPEnemyCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ACPEnemyCharacter();

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    virtual void Landed(const FHitResult& Hit) override;

    UFUNCTION(BlueprintPure, Category = "Enemy|State")
    bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintPure, Category = "Enemy|State")
    float GetHealth() const;

    // Enemy 체력을 0.0~1.0 범위에서 감소시킨다.
    UFUNCTION(BlueprintCallable, Category = "Enemy|State")
    float ApplyDamage(float DamageAmount, AActor* DamageCauser);

    // 거리, 가림, 체력, 쿨타임 등을 확인
    UFUNCTION(BlueprintPure, Category = "Enemy|Attack")
    bool CanAttackTarget(AActor* Target) const;

    // 공격 시작에 성공했는지 반환
    UFUNCTION(BlueprintCallable, Category = "Enemy|Attack")
    bool TryStartAttack(AActor* Target);

    // 공격 몽타주의 Notify에서 호출
    UFUNCTION(BlueprintCallable, Category = "Enemy|Attack")
    void ApplyAttackHit();

    // 상태 이탈 또는 슬라임 처치 시 호출
    UFUNCTION(BlueprintCallable, Category = "Enemy|Attack")
    void CancelAttack();

    bool IsAttacking() const { return bIsAttacking; }
    
    bool DidLastAttackCompleteNormally() const
    {
        return bLastAttackCompletedNormally;
    }
    
    bool IsVictoryPlaying() const { return bIsVictoryPlaying; }
    
    bool DidVictoryCompletedNormally() const
    {
        return bVictoryCompletedNormally;
    }
    
    bool IsVictoryRequested() const { return bVictoryRequested; }
    
    UFUNCTION(BlueprintCallable, Category = "Enemy|Attack")
    void RequestVictory();
    
    void ClearVictoryRequest();
    
    UFUNCTION(BlueprintCallable, Category = "Enemy|Victory")
    bool StartVictoryRequest();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    void HandleDeath(AActor* DamageCauser);

    void HandleHealthChanged(const FOnAttributeChangeData& ChangeData);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
    TObjectPtr<UCPEnemyAttributeSet> EnemyAttributeSet;

    // 사망 후 Mesh를 움직이는 구형 물리 충돌체다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
    TObjectPtr<USphereComponent> DeathPhysicsSphere;

    // Enemy BP에서 포션 반응 GA를 등록한다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Abilities")
    TArray<TSubclassOf<UGameplayAbility>> PotionReactionAbilities;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack")
    TArray<TObjectPtr<UAnimMontage>> AttackMontages;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack")
    TArray<TObjectPtr<UAnimMontage>> VictoryMontages;

    // 체력이 0이 되었을 때 유지할 사망 Pose. Enemy BP에서 지정한다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Death")
    TObjectPtr<UAnimSequence> DeathPoseAnimation;

    // 액터 중심 사이의 3D 거리 기준(높이 포함)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "1.0", Units = "cm"))
    float AttackRange = 100.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.0"))
    float AttackDamage = 10.f;

    // 공격 쿨타임
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.0", Units = "s"))
    float AttackCooldown = 1.f;

    // 실제 타격 시 정면 좌우 허용 각도
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.0", ClampMax = "180.0", Units = "deg"))
    float AttackHalfAngle = 60.f;

private:
    void GrantPotionReactionAbilities();
    bool IsTargetInAttackReach(AActor* Target) const;
    void HandleAttackBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    void HandleAttackEnded(UAnimMontage* Montage, bool bInterrupted);
    void PlayDeathPose();
    void StartDeathPhysics(const FVector& DeathVelocity, const FVector& SurfaceNormal);
    void HandleVictoryEnded(UAnimMontage* Montage, bool bInterrupted);

    UPROPERTY()
    TWeakObjectPtr<AActor> AttackTarget;

    bool bIsAttacking = false;
    bool bAttackHitConsumed = false;
    bool bAttackHitEnabled = false;
    bool bLastAttackCompletedNormally = false;
    bool bIsDead = false;

    FDelegateHandle HealthChangedDelegateHandle;
    TWeakObjectPtr<AActor> PendingDamageCauser;

    FRotator InitialMeshRelativeRotation = FRotator::ZeroRotator;
    FVector LandingVelocity = FVector::ZeroVector;
    FVector LandingSurfaceNormal = FVector::UpVector;
    bool bHandlingLanding = false;

    UPROPERTY(Transient)
    TObjectPtr<UAnimMontage> ActiveVictoryMontage;

    bool bIsVictoryPlaying = false;
    bool bVictoryCompletedNormally = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|Victory", meta = (AllowPrivateAccess = "true"))
    bool bVictoryRequested = false;

    double NextAttackAllowedTime = 0.0;
    int32 RandomMontageNum = 0;
};
