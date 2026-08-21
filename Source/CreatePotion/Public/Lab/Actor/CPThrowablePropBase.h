// CPThrowablePropBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Interface/CPInteractable.h"
#include "GameFramework/Actor.h"
#include "CPThrowablePropBase.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;

UENUM(BlueprintType)
enum class ECPThrowablePropState : uint8
{
    Resting,
    Held,
    Thrown
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCPOnThrowablePropStateChanged, ECPThrowablePropState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCPOnThrowablePropThrown, AActor*, Thrower);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCPOnThrowablePropHit, AActor*, OtherActor, const FHitResult&, HitResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCPOnThrowablePropRested);

UCLASS(Abstract)
class CREATEPOTION_API ACPThrowablePropBase : public AActor, public ICPInteractable
{
    GENERATED_BODY()

public:
    ACPThrowablePropBase();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual bool CanInteract_Implementation(AActor* Interactor) override;
    virtual FName GetInteractionName_Implementation() override;

    bool AttachAsHeld(USceneComponent* CarryAnchor);
    void DetachAsHeld(const FVector& DropLocation);

    // 머리 위에서 분리하고 물리 투척
    bool Throw(const FVector& Direction, float Speed);

    UFUNCTION(BlueprintPure, Category = "Prop|State")
    ECPThrowablePropState GetPropState() const;

    UFUNCTION(BlueprintPure, Category = "Prop|State")
    bool IsHeld() const;

    UFUNCTION(BlueprintPure, Category = "Prop|State")
    bool IsThrown() const;

    UFUNCTION(BlueprintPure, Category = "Prop|State")
    bool IsResting() const;

    UFUNCTION(BlueprintPure, Category = "Prop|Throw")
    AActor* GetLastThrower() const;

public:
    UPROPERTY(BlueprintAssignable, Category = "Prop|Event")
    FCPOnThrowablePropStateChanged OnPropStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Prop|Event")
    FCPOnThrowablePropThrown OnPropThrown;

    /*
     * 물리 충돌이 발생할 때 알림.
     * 포션의 첫 Impact 여부는 PotionImpactComponent가 판단.
     */
    UPROPERTY(BlueprintAssignable, Category = "Prop|Event")
    FCPOnThrowablePropHit OnPropHit;

    UPROPERTY(BlueprintAssignable, Category = "Prop|Event")
    FCPOnThrowablePropRested OnPropRested;

protected:
    /*
     * PotionActor 전용, 상속 받고 재정의
     * PotionImpactComponent::EnableImpactProcessing()을 호출.
     */
    virtual void HandleThrowStarted(AActor* Thrower);

    /*
     * PotionActor 전용, 상속 받고 재정의
     * PotionImpactComponent::TryTriggerPotionImpact()를 호출.
     */
    virtual void HandleThrownImpact(AActor* OtherActor, const FHitResult& HitResult);

    void SetPropState(ECPThrowablePropState NewState);
    void StartRestCheck();
    void StopRestCheck();
    void CheckForRest();

    UFUNCTION()
    void HandleMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& HitResult);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prop")
    TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

    // --------- 투척물의 상태 관리 --------- //
    // Prop마다 머리 위 위치와 회전을 보정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop|Carry")
    FTransform HeldRelativeTransform;

    // 20.0만큼 투척방향으로 이동(물리 활성화 전)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop|Throw", meta = (ClampMin = "0.0", Units = "cm"))
    float ThrowStartOffset = 20.f;

    // 0.1초마다 정지 상태 확인
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop|Rest", meta = (ClampMin = "0.01", Units = "s"))
    float RestCheckInterval = 0.1f;

    // 투척 후 선형 이동 감쇠
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop|Physics", meta = (ClampMin = "0.0"))
    float ThrowLinearDamping = 1.5f;

    // 투척 후 회전 감쇠
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop|Physics", meta = (ClampMin = "0.0"))
    float ThrowAngularDamping = 5.f;
    
    // 30.0의 선속도 이하면 정지상태로 인정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop|Rest", meta = (ClampMin = "0.0", Units = "cm/s"))
    float RestLinearSpeedThreshold = 30.f;

    // 45.0의 각속도 이하면 정지상태로 인정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop|Rest", meta = (ClampMin = "0.0", Units = "deg/s"))
    float RestAngularSpeedThreshold = 45.f;

    // 멈춘 시간 0.2초까지는 정지상태로 인정하지 않음
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop|Rest", meta = (ClampMin = "0.0", Units = "s"))
    float RestConfirmationDuration = 0.2f;

private:
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Prop|State", meta = (AllowPrivateAccess = "true"))
    ECPThrowablePropState PropState = ECPThrowablePropState::Resting;

    UPROPERTY()
    TObjectPtr<AActor> LastThrower;

    bool bHasHitSinceThrow = false;
    float LowSpeedElapsedTime = 0.f;

    FTimerHandle RestCheckTimerHandle;
};
