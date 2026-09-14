#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "StateTreeTaskBase.h"
#include "CPSTTask_Attack.generated.h"

class ACPEnemyCharacter;

// 공격 가능 조건에서 사용할 데이터
USTRUCT()
struct CREATEPOTION_API FCPSTCondition_CanAttackInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Context")
    TObjectPtr<ACPEnemyCharacter> Enemy = nullptr;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> TargetActor = nullptr;
};

USTRUCT(meta = (DisplayName = "CP Can Attack", Category = "Enemy"))
struct CREATEPOTION_API FCPSTCondition_CanAttack : public FStateTreeConditionCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FCPSTCondition_CanAttackInstanceData;

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

// 공격 Task에서 사용할 데이터
USTRUCT()
struct CREATEPOTION_API FCPSTTask_AttackInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Context")
    TObjectPtr<ACPEnemyCharacter> Enemy = nullptr;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> TargetActor = nullptr;

    UPROPERTY(Transient)
    bool bStartedAttack = false;
};

USTRUCT(meta = (DisplayName = "CP Attack", Category = "Enemy"))
struct CREATEPOTION_API FCPSTTask_Attack : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FCPSTTask_AttackInstanceData;

    FCPSTTask_Attack()
    {
        // 공격 중에만 종료 여부를 확인
        bShouldCallTick = true;

        // 이번 공격의 대상은 진입 시 받아온 값 유지
        bShouldCopyBoundPropertiesOnTick = false;
        bShouldCopyBoundPropertiesOnExitState = false;
    }

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};