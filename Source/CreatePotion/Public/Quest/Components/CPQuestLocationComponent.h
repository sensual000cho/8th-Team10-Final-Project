// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quest/QuestTypes.h"
#include "CPQuestLocationComponent.generated.h"

class UQuestManager;

// 위치 변경 알림 (true: 원래 위치를 벗어남 / false: 원래 위치로 복귀)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuestLocationChanged, bool /*bAwayFromHome*/);

DECLARE_MULTICAST_DELEGATE(FOnQuestLocationFadeFinished);

// 퀘스트 상태별 위치 규칙 하나
USTRUCT(BlueprintType)
struct FCPQuestLocationRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Quest|Location")
	FName QuestID;

	// 이 상태일 때 Destination으로 이동
	UPROPERTY(EditAnywhere, Category = "Quest|Location")
	EQuestState TriggerState = EQuestState::Accepted;

	// 레벨에 배치한 TargetPoint 등
	UPROPERTY(EditAnywhere, Category = "Quest|Location")
	TObjectPtr<AActor> Destination;
};

// 퀘스트 상태에 따라 소유 액터(NPC)의 위치를 결정하는 컴포넌트
// - 조건이 맞는 규칙 중 목록 가장 아래 것이 적용됨
// - 맞는 규칙이 없으면 레벨 배치 위치로 복귀
// - SetRefreshLocked(true) 동안에는 이동을 보류했다가, 해제 시 한 번에 적용
// - 이동 시 화면 페이드 아웃 → 이동 → 페이드 인 (레벨 진입 시에는 즉시 이동)
UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPQuestLocationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPQuestLocationComponent();

	// 현재 퀘스트 상태 기준으로 위치 재적용
	UFUNCTION(BlueprintCallable, Category = "Quest|Location")
	void RefreshLocation();

	// 대화 중 등 이동을 잠시 막아야 할 때 사용
	void SetRefreshLocked(bool bLocked);

	// 원래 위치를 벗어나 퀘스트 위치에 있는지
	bool IsAwayFromHome() const { return CurrentDestination != nullptr; }

	FOnQuestLocationChanged OnLocationChanged;
	FOnQuestLocationFadeFinished OnFadeFinished;

	// 페이드 연출이 진행 중인지
	bool IsFading() const { return bFading; }

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 레벨에 배치된 인스턴스에서 설정
	UPROPERTY(EditInstanceOnly, Category = "Quest|Location")
	TArray<FCPQuestLocationRule> LocationRules;

	// 이동 시 화면 페이드 사용 여부
	UPROPERTY(EditAnywhere, Category = "Quest|Location|Fade")
	bool bUseFade = true;

	// 페이드 아웃/인 각각의 시간(초)
	UPROPERTY(EditAnywhere, Category = "Quest|Location|Fade", meta = (ClampMin = "0.05", ClampMax = "2.0"))
	float FadeDuration = 0.3f;

private:
	UFUNCTION()
	void HandleQuestUpdated(FName QuestID, EQuestState NewState);

	UQuestManager* GetQuestManager() const;

	// 현재 퀘스트 상태에 맞는 목적지 계산 (없으면 nullptr = 원래 위치)
	AActor* ResolveDestination() const;

	// 실제 이동 수행 (페이드와 무관)
	void ApplyLocation(AActor* Target);

	// 페이드 중간(화면이 어두운 시점)에 호출
	void OnFadeOutFinished();
	void OnFadeInFinished();

	APlayerCameraManager* GetCameraManager() const;

	// 레벨 배치 위치 (MoveComponent의 위치 복원보다 먼저 기록)
	FTransform HomeTransform;

	// 현재 적용 중인 목적지 (nullptr = 원래 위치)
	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentDestination;

	// 페이드 도중 이동할 목적지
	UPROPERTY(Transient)
	TObjectPtr<AActor> PendingDestination;

	bool bRefreshLocked = false;
	bool bPendingRefresh = false;

	// 페이드 진행 중 여부 (진행 중에는 새 요청을 미룸)
	bool bFading = false;
	bool bRefreshAfterFade = false;

	FTimerHandle FadeTimerHandle;
};