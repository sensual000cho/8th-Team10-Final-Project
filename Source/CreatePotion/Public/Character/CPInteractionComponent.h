// CPInteractionComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPromptChanged, FText, Prompt, FName, TargetName);

// 상호작용 프로그레스 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionProgressChanged, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionCompleted);

class UCameraComponent;
class UInputAction;
class UInputMappingContext;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
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

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TraceDistance = 200.f; // 감지 거리
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get();}

private:
	void PerformTrace(); // 타이머로 주기 실행
	void ClearCurrentTarget();
	
	// --- Timed Interaction ---
	void StartTimedInteraction(AActor* Target, float Duration);
	void UpdateTimedInteraction();
	void CompleteTimedInteraction();
	
private:
	// --- 상호작용 대상 Highlight ---
	void SetActorHighlight(AActor* Target, bool bHighlighted);
	
	UPROPERTY(EditDefaultsOnly, Category = "Highlight")
	UMaterialInterface* HighlightMaterial;
	
private:
	// 상호작용 대상 탐색 타이머
	FTimerHandle TraceTimerHandle;
	// 시간형 상호작용 진행 타이머
	FTimerHandle InteractionTimerHandle;

	// 현재 조준 중인 대상
	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentTarget;
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

private:
	// GA Event를 전달할때 사용하는 헬퍼 함수
	bool RequestPickupAbility(AActor* Target);

#pragma region InputMappingContext
public:
	UPROPERTY(EditAnywhere, Category = "Interaction|IMC")
	UInputMappingContext* InteractionMappingContext;

	UPROPERTY(EditAnywhere, Category = "Interaction|Input")
	UInputAction* InteractAction;

private:
	void TryBindInputMappingContext();	// 추가로 IMC를 바인딩 하는 것을 시도할 함수

private:
	int32 BindRetryCount = 0;

	FTimerHandle IMCBindingTimerHandle;		// IMC를 바인딩 할 타이머 핸들
#pragma endregion
};
