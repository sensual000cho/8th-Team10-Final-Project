// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CPInteractableComponent.generated.h"

enum class ECPInteractionDisplayState : uint8;
class UUserWidget;
class UWidgetComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPInteractableComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// lifeCycle
	UCPInteractableComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// 플레이어가 시선을 맞췄을 때 호출
	virtual void BeginFocus(AActor* Interactor);
	
	// 플레이어가 시선을 거뒀을 때 호출
	virtual void EndFocus(AActor* Interactor);
	
	// 상호작용 가능 여부 반환
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual bool CanInteract() const;
	
	// 상호작용 실행
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual bool TryInteract(AActor* Interactor); 
	
	// 시간형 상호작용 실행
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void BeginTimeInteraction(AActor* Interactor);
	
	// UI 갱신
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RefreshUI();
	
	// --- Getter---
	UFUNCTION(BlueprintPure, Category = "Interaction")
	virtual FName GetTargetName() const { return TargetName; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	virtual FText GetInteractionPrompt() const { return InteractionPrompt; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	virtual float GetInteractionDuration() const { return InteractionDuration; }
	
private:
	void UpdateWidgetLocation(AActor* Interactor);

protected:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName TargetName; // 예: "약초", "주민", "보물상자"
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractionPrompt; // 예: "채집하기", "대화하기", "열기"
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Timed")
	float InteractionDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (DisplayName = "Widget Class"))
	TSubclassOf<UUserWidget> InteractionWidgetClass;
	
	UPROPERTY(Transient)
	TObjectPtr<UWidgetComponent> InteractionWidgetComp;
	
private:
	UPROPERTY()
	TObjectPtr<AActor> FocusedInteractor;
};
