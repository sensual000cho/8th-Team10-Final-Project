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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionProgressChanged, float, Progress);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionAlignmentStarted);

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
	
	// PromptWidget
	UPROPERTY(BlueprintAssignable)
	FOnPromptChanged OnPromptChanged;
	
	// ProgressBar, PlayerBP
	UPROPERTY(BlueprintAssignable)
	FOnInteractionStarted OnInteractionStarted;
	
	// ProgressBar
	UPROPERTY(BlueprintAssignable)
	FOnInteractionProgressChanged OnInteractionProgressChanged;
	
	// ProgressBar, PlayerBP
	UPROPERTY(BlueprintAssignable)
	FOnInteractionCompleted OnInteractionCompleted;
	
	// ProgressBar, PlayerBP
	UPROPERTY(BlueprintAssignable)
	FOnInteractionCancelled OnInteractionCancelled;
	
	// 시간형 상호작용 시작 전 대상 방향으로 회전 시작, PlayerBP
	UPROPERTY(BlueprintAssignable)
	FOnInteractionAlignmentStarted OnInteractionAlignmentStarted;
	
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
	
	// 시간형 상호작용 전에 대상을 향해 회전 중인지 확인
	UFUNCTION(BlueprintPure, Category = "Interaction|Timed")
	bool IsTimedInteractionAligning() const;
	
	// 현재 고정된 InteractingTarget을 바라보는 회전값
	UFUNCTION(BlueprintPure, Category = "Interaction|Timed")
	FRotator GetTimedInteractionFacingRotation() const;

	// 캐릭터가 사용할 회전 속도
	UFUNCTION(BlueprintPure, Category = "Interaction|Timed")
	float GetInteractionFacingRotationSpeed() const;

	// 캐릭터가 목표 방향까지 회전한 뒤 호출
	UFUNCTION(BlueprintCallable, Category = "Interaction|Timed")
	bool FinishTimedInteractionAlignment();
	
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
	
	// 정렬 중 대상, 이동, 시간 초과만 검사
	void ValidateTimedInteractionAlignment();
	// 회전 후 실제 몽타주와 프로그레스 시작
	void BeginTimedInteraction();
	// Yaw 회전값 계산
	FRotator MakeInteractionFacingRotation(const AActor* Target) const;
	
	// --- 상호작용 대상 Highlight ---
	void SetActorHighlight(AActor* Target, bool bHighlighted);
	
	// UI Refresh 함수
	void RefreshCurrentTargetPresentation();
	
	// 상호작용 취소 함수
	void CancelTimedInteraction();
	bool HasInteractorMoved() const;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Highlight")
	UMaterialInterface* HighlightMaterial;
	
	// 상호작용 대상 탐색 타이머
	FTimerHandle TraceTimerHandle;
	// 시간형 상호작용 진행 타이머
	FTimerHandle InteractionTimerHandle;
	// 대상 방향으로 회전할 때 사용하는 타이머
	FTimerHandle InteractionAlignmentTimerHandle;
	
	// --- 회전 관련 변수들 ---
	// 정렬 상태 검증 주기. 회전에는 사용하지 않음.
	UPROPERTY(EditAnywhere, Category = "Interaction|Timed|Alignment", meta = (ClampMin = "0.01", Units = "s"))
	float InteractionAlignmentValidationInterval = 0.05f;

	// 캐릭터의 RInterp To Constant에서 사용할 초당 회전 속도
	UPROPERTY(EditAnywhere, Category = "Interaction|Timed|Alignment", meta = (ClampMin = "0.0", Units = "deg/s"))
	float InteractionFacingRotationSpeed = 540.f;

	// 이 각도 이내면 회전 완료
	UPROPERTY(EditAnywhere, Category = "Interaction|Timed|Alignment", meta = (ClampMin = "0.0", Units = "deg"))
	float InteractionFacingTolerance = 3.f;

	// BP 연결 실패 등의 상황에서 무한 정렬 상태 방지
	UPROPERTY(EditAnywhere, Category = "Interaction|Timed|Alignment", meta = (ClampMin = "0.1", Units = "s"))
	float InteractionAlignmentTimeout = 2.f;

	UPROPERTY(VisibleInstanceOnly, Category = "Interaction|Timed|Alignment")
	bool bTimedInteractionAligning = false;
	
	// Actor가 목표 방향에 도착한 후 Root Offset이 Release될 시간
	UPROPERTY(EditAnywhere, Category = "Interaction|Timed|Alignment", meta = (ClampMin = "0.0", Units = "s"))
	float InteractionFacingReleaseDuration = 0.35f;

	float InteractionAlignmentElapsedTime = 0.f;

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
	
	// 시간형 상호작용을 시작한 위치
	FVector InteractionStartLocation = FVector::ZeroVector;

	// 이 거리 이상 이동하면 상호작용 취소
	UPROPERTY(EditAnywhere, Category = "Interaction|Timed", meta = (ClampMin = "0.0", Units = "cm"))
	float InteractionCancelDistance = 10.f;

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
