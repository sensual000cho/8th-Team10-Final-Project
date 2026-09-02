// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPInteractableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteracted, AActor*, Interactor);

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
	
	// --- Getter 함수 ---
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool CanInteract() const { return bCanInteract; }
	
	// UFUNCTION(BlueprintPure, Category = "Interaction")
	// FName GetInteractionName() const { return InteractionName; }
	//
	// UFUNCTION(BlueprintPure, Category = "Interaction")
	// FText GetInteractionPrompt() const { return InteractionPrompt; }
	//
public:
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteracted OnInteracted;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Timed")
	float InteractionDuration = 0.f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanInteract = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName InteractionName; // 예: "약초", "주민", "보물상자"
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractionPrompt; // 예: "채집하기", "대화하기", "열기"
	
private:
	UPROPERTY()
	UWidgetComponent* InteractWidgetComp;
};
