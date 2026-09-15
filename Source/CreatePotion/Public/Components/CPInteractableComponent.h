// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPInteractableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorInteractionStarted, AActor*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorInteractionCompleted, AActor*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActorInteractionCancelled);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractableAlignmentStarted);

enum class ECPInteractionDisplayState : uint8;
class UCPInteractionComponent;
class UWidgetComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPInteractableComponent();
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// 플레이어가 시선을 맞췄을 때 호출
	virtual void BeginFocus(AActor* Interactor);
	
	// 플레이어가 시선을 거뒀을 때 호출
	virtual void EndFocus(AActor* Interactor);
	
	// 상호작용 실행
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void HandleInteract(AActor* Interactor); 
	
	// 시간형 상호작용 실행
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HandleInteractionStarted(AActor* Interactor);
	
	// UI 갱신
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RefreshUI();
	
	
	
	// --- Setter ---
	
	// 활성화/비활성화/숨김 여부 Enum값 설정
	// Enabled: 활성화(UI, 하이라이트 보임), Disabled: 비활성화(회색 UI만 보임), Hidden: 모두 숨김
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void SetDisplayState(ECPInteractionDisplayState NewDisplayState);
	
	// 상호작용 가능 여부 설정 (기존의 CanInteract() 함수와 같은 역할)
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void SetInteractability(bool bValue);
	
	// 상호작용 프롬프트 설정
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void SetInteractionPrompt(FText NewPrompt);
	
	// 상호작용 대상 액터 이름 설정
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void SetTargetName(FName NewName);
	
	// 시간형 상호작용 소요시간 설정
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void SetInteractionDuration(float NewDuration);
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionWidgetComponent(UWidgetComponent* InWidgetComp);
	
	
	// --- Getter---
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	virtual bool CanInteract() const { return bCanInteract; }
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	ECPInteractionDisplayState GetDisplayState() const { return CachedDisplayState; }
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	virtual bool ShouldShowUnavailableInteraction() const { return bShouldShowUnavailableInteraction; }
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FName GetTargetName() const { return TargetName; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetInteractionPrompt() const { return InteractionPrompt; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetInteractionDuration() const { return InteractionDuration; }
	
private:
	void UpdateInteractionWidgetLocation(AActor* Interactor);

public:
	// 상호작용 완료 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "InteractableComponent")
	FOnActorInteractionCompleted OnActorInteractionCompleted;
	
	// 상호작용 시작 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "InteractableComponent")
	FOnActorInteractionStarted OnActorInteractionStarted;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanInteract = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bShouldShowUnavailableInteraction = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName TargetName; // 예: "약초", "주민", "보물상자"
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractionPrompt; // 예: "채집하기", "대화하기", "열기"
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	ECPInteractionDisplayState CachedDisplayState; // 활성화/비활성화/숨기기 상태 캐싱
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Timed")
	float InteractionDuration = 0.f;
	
	UPROPERTY()
	UWidgetComponent* InteractWidgetComp;
	
private:	
	UPROPERTY()
	UCPInteractionComponent* InteractionComp;
	
	UPROPERTY()
	TObjectPtr<AActor> FocusedInteractor;
	
	FVector RelativeLocation;
};
