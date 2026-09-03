// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPInteractableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteracted, AActor*, Interactor);

enum class ECPInteractionDisplayState : uint8;
class UWidgetComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPInteractableComponent();
	
	virtual void BeginPlay() override;
	
	// 플레이어가 시선을 맞췄을 때 호출
	void BeginFocus(AActor* Interactor);
	
	// 플레이어가 시선을 거뒀을 때 호출
	void EndFocus(AActor* Interactor);
	
	// 상호작용 실행
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor); 
	
	// UI 갱신
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RefreshUI();
	
	// --- Setter ---
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetDisplayState(ECPInteractionDisplayState NewDisplayState);
	
	
	// --- Getter---
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool CanInteract() const { return bCanInteract; }
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool ShouldShowUnavailableInteraction() const { return bShouldShowUnavailableInteraction; }
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FName GetTargetName() const { return TargetName; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetInteractionPrompt() const { return InteractionPrompt; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetInteractionDuration() const { return InteractionDuration; }

public:
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteracted OnInteracted;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Timed")
	float InteractionDuration = 0.f;

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
	
private:
	UPROPERTY()
	UWidgetComponent* InteractWidgetComp;
};
