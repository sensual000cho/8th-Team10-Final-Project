// CPCarryComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CPCarryComponent.generated.h"

class AActor;
class ACPThrowablePropBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCPOnCarryHeldPropChanged, ACPThrowablePropBase*, HeldProp);

/**
 * 플레이어가 들고 있는 공용 기능(부착, 투척, 해제)
 *
 * Ingredient와 Potion 모두 ACPThrowablePropBase를 통해 동일하게 처리.
 *
 * 실제 입력과 상태 전환은 Gameplay Ability가 담당하고,
 * 지속적으로 유지해야 하는 Held 참조와 물리 처리는 현재 컴포넌트가 담당.
 */
UCLASS(ClassGroup = (Carry), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPCarryComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UCPCarryComponent();

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // 현재 Prop을 부착할 수 있는지 확인
    UFUNCTION(BlueprintPure, Category = "Carry")
    bool CanAttachProp(ACPThrowablePropBase* Prop) const;

    // Prop을 이 컴포넌트 위치에 부착하고 Held 참조로 등록
    UFUNCTION(BlueprintCallable, Category = "Carry")
    bool AttachProp(ACPThrowablePropBase* Prop);
    
    // 기존 Held Prop을 Drop한 뒤 새로운 Prop을 부착
    UFUNCTION(BlueprintCallable, Category = "Carry")
    bool ReplaceHeldProp(ACPThrowablePropBase* NewProp);

    // 지정한 Held Prop을 분리하고 월드 위치에 배치
    UFUNCTION(BlueprintCallable, Category = "Carry")
    bool DetachProp(ACPThrowablePropBase* Prop, const FVector& DropLocation);

    // 현재 Held Prop을 지정한 월드 위치에 내려놓음
    UFUNCTION(BlueprintCallable, Category = "Carry")
    bool DropHeldProp(const FVector& DropLocation);

    // 현재 Held Prop을 지정한 방향과 속도로 투척
    // 새로운 GA_ThrowThrowable은 이 함수를 사용한다.
    UFUNCTION(BlueprintCallable, Category = "Carry|Throw")
    bool ThrowHeldProp(const FVector& Direction, float Speed);

    // Held 상태를 강제로 초기화
    // 캐릭터 리셋이나 비정상적인 Ability 취소 복구 시 사용
    UFUNCTION(BlueprintCallable, Category = "Carry")
    void ResetCarryState();

    UFUNCTION(BlueprintPure, Category = "Carry")
    bool HasHeldProp() const;

    UFUNCTION(BlueprintPure, Category = "Carry")
    ACPThrowablePropBase* GetHeldProp() const;

    /*
     * 기존 임시 투척 함수.
     *
     * 기존 BP와 코드의 호환성을 위해 당장은 유지(추후 삭제).
     * 새로운 GA 기반 Carry 시스템에서는 사용하지 않음.
     */
    UFUNCTION(BlueprintCallable, Category = "Carry|Legacy")
    bool TryThrowHeldAlchemyProp(float ThrowSpeed = 800.f, float UpwardBias = 0.2f);

public:
    // 현재 들고 있는 Prop이 변경되었음을 알림
    UPROPERTY(BlueprintAssignable, Category = "Carry")
    FCPOnCarryHeldPropChanged OnHeldPropChanged;

private:
    FVector MakeReplacementDropLocation() const;
    
    // Held 참조 변경과 OnDestroyed 바인딩을 한곳에서 처리
    void SetHeldProp(ACPThrowablePropBase* NewHeldProp);

    // 들고 있는 Prop이 외부에서 파괴됐을 때 참조 정리
    UFUNCTION()
    void HandleHeldPropDestroyed(AActor* DestroyedActor);

private:
    UPROPERTY(EditAnywhere, Category = "Carry|Drop", meta = (ClampMin = "0.0", Units = "cm"))
    float ReplacementDropForwardDistance;
    
    // ResetCarryState 호출 시 캐릭터 전방에 내려놓을 거리
    UPROPERTY(EditAnywhere, Category = "Carry", meta = (ClampMin = "0.0"))
    float ResetDropForwardDistance;

    UPROPERTY(VisibleInstanceOnly, Category = "Carry")
    TObjectPtr<ACPThrowablePropBase> HeldProp;
};
