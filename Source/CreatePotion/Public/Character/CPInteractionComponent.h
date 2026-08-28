// CPInteractionComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPInteractionComponent.generated.h"

UENUM(BlueprintType)
enum class ECPInteractionDisplayState : uint8
{
	Hidden,   // UI 없음, 하이라이트 없음
	Disabled, // 회색 UI, 하이라이트 없음
	Enabled   // 정상 UI, 노란 하이라이트
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPromptChanged, FText, Prompt, FName, TargetName, ECPInteractionDisplayState, DisplayState);

// 상호작용 프로그레스 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionProgressChanged, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionCompleted);

class AActor;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UMaterialInterface;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPInteractionComponent();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void TryInteract();
	
	UPROPERTY(BlueprintAssignable)
	FOnPromptChanged OnPromptChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnInteractionStarted OnInteractionStarted;
	
	UPROPERTY(BlueprintAssignable)
	FOnInteractionProgressChanged OnInteractionProgressChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnInteractionCompleted OnInteractionCompleted;

	// 실제 상호작용 거리
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Trace", meta = (ClampMin = "0.0", Units = "cm"))
	float TraceDistance = 200.f;
	
	// (카메라 - 지점) 최대거리
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Trace", meta = (ClampMin = "0.0", Units = "cm"))
	float CameraTraceDistance = 5000.f;

	// 실제 캐릭터 상호작용 거리를 측정 할 시작점(캐릭터 위치 + Offset)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Trace", meta = (Units = "cm"))
	float TraceHeightOffset = 70.f;

	// Sphere 반지름(범위 내 삭호작용 판별)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Trace", meta = (ClampMin = "0.0", Units = "cm"))
	float InteractionSweepRadius = 50.f;

	// 상호작용 대상 탐색 주기
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Trace", meta = (ClampMin = "0.02", Units = "s"))
	float TraceUpdateInterval = 0.08f;

	// 후보 탐색에 포함할 Object Type
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Trace")
	TArray<TEnumAsByte<EObjectTypeQuery>> InteractionObjectTypes;

	// 카메라 정면 정렬 점수의 비중
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Selection", meta = (ClampMin = "0.0"))
	float AlignmentScoreWeight = 0.8f;

	// 캐릭터와 후보 사이 거리 점수의 비중
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Selection", meta = (ClampMin = "0.0"))
	float DistanceScoreWeight = 0.2f;

	// 현재 선택 중인 대상을 유지하기 위한 추가 점수
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Selection", meta = (ClampMin = "0.0"))
	float CurrentTargetScoreBonus = 0.1f;

	// 이 값보다 카메라 정면에서 벗어난 후보는 제외
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Selection", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float MinimumCameraAlignment = 0.5f;
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get();}

private:
	void PerformTrace(); // 타이머로 주기 실행
	
	// 상호작용 보완 함수들
	AActor* SelectBestInteractionTarget(const TArray<FHitResult>& InteractionHits, const FVector& CameraLocation, const FVector& CameraForward, ECPInteractionDisplayState& OutDisplayState) const;
	ECPInteractionDisplayState ResolveInteractionDisplayState(AActor* Target, AActor* Interactor) const;
	
	FVector GetInteractionFocusLocation(const AActor* Target) const;
	bool HasLineOfSightToTarget(const AActor* Target, const FVector& CameraLocation, const FVector& TargetLocation) const;
	float CalculateInteractionScore(const FHitResult& HitResult, const FVector& TargetLocation, const FVector& CameraLocation, const FVector& CameraForward) const;
	
	void UpdateCurrentTarget(AActor* NewTarget, ECPInteractionDisplayState NewDisplayState);
	void ClearCurrentTarget();
	
	// --- Timed Interaction ---
	void StartTimedInteraction(AActor* Target, float Duration);
	void UpdateTimedInteraction();
	void CompleteTimedInteraction();
	
	// --- 상호작용 대상 Highlight ---
	void SetActorHighlight(AActor* Target, bool bHighlighted);
	
	// UI Refresh 함수
	void RefreshCurrentTargetPresentation();
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Highlight")
	UMaterialInterface* HighlightMaterial;
	
	// 상호작용 대상 탐색 타이머
	FTimerHandle TraceTimerHandle;
	// 시간형 상호작용 진행 타이머
	FTimerHandle InteractionTimerHandle;

	// 현재 조준 중인 대상
	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentTarget;
	
	// 현재 Display 상태
	UPROPERTY(VisibleInstanceOnly, Category = "Interaction")
	ECPInteractionDisplayState CurrentDisplayState = ECPInteractionDisplayState::Hidden;
	
	// 현재 시간형 상호작용 중인 대상
	UPROPERTY()
	TWeakObjectPtr<AActor> InteractingTarget;

	UPROPERTY()
	TObjectPtr<UCameraComponent> Camera;

	// 시간형 상호작용에 필요한 전체 시간
	float InteractionDuration = 0.f;
	// 현재까지 진행된 시간
	float InteractionElapsedTime = 0.f;
	// 진행도 갱신 주기
	float InteractionUpdateInterval = 0.05f;

#pragma region InputMappingContext
public:
	UPROPERTY(EditAnywhere, Category = "Interaction|IMC")
	UInputMappingContext* InteractionMappingContext;

	UPROPERTY(EditAnywhere, Category = "Interaction|Input")
	UInputAction* InteractAction;

private:
	void TryBindInputMappingContext();	// 추가로 IMC를 바인딩 하는 것을 시도할 함수
	
	int32 BindRetryCount = 0;
	FTimerHandle IMCBindingTimerHandle;		// IMC를 바인딩 할 타이머 핸들
#pragma endregion
};
