// CPInteractionComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPromptChanged, FText, Prompt);
class UCameraComponent;

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

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TraceDistance = 200.f; // 감지 거리
	
private:
	void PerformTrace(); // 타이머로 주기 실행
	void ClearCurrentTarget();
	
	// --- Timed Interaction ---
	void StartTimedInteraction(AActor* Target, float Duration);
	void UpdateTimedInteraction();
	void CompleteTimedInteraction();
	
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
};
