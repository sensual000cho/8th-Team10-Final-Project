// CPEnemyCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CPEnemyCharacter.generated.h"

class UAnimMontage;

UCLASS()
class CREATEPOTION_API ACPEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ACPEnemyCharacter();

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

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack")
    TArray<TObjectPtr<UAnimMontage>> AttackMontages;

    // 액터 중심 사이의 3D 거리 기준(높이 포함)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "1.0", Units = "cm"))
    float AttackRange = 110.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.0"))
    float AttackDamage = 10.f;

    // 공격 쿨타임
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.0", Units = "s"))
    float AttackCooldown = 1.f;

    // 실제 타격 시 정면 좌우 허용 각도
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.0", ClampMax = "180.0", Units = "deg"))
    float AttackHalfAngle = 50.f;

private:
    bool IsTargetInAttackReach(AActor* Target) const;
    void HandleAttackBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    void HandleAttackEnded(UAnimMontage* Montage, bool bInterrupted);

    UPROPERTY()
    TWeakObjectPtr<AActor> AttackTarget;

    bool bIsAttacking = false;
    bool bAttackHitConsumed = false;
    bool bAttackHitEnabled = false;
    bool bLastAttackCompletedNormally = false;

    double NextAttackAllowedTime = 0.0;
    int32 RandomMontageNum = 0;
};
