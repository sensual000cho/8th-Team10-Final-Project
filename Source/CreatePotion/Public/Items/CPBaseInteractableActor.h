// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPBaseInteractableActor.generated.h"

class UCPInteractableComponent;
class UWidgetComponent;
enum class ECPInteractionDisplayState : uint8;

UCLASS()
class CREATEPOTION_API ACPBaseInteractableActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACPBaseInteractableActor();

	virtual void PostInitializeComponents() override;
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CheckInteractionConditions();

protected:
	// Called when the game starts or when spawned
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- 상호작용 오버라이드 함수 ---
	// 실제 상호작용 실행
	UFUNCTION(BlueprintCallable, Category = "Interaction|InteractableActor")
	virtual void HandleInteract(AActor* Interactor);
	
	// 시간형 상호작용 실행 (ex: 애니메이션 몽타주 재생 등)
	UFUNCTION(BlueprintCallable, Category = "Interaction|InteractableActor")
	virtual void HandleInteractionStarted(AActor* Interactor);
	
	// 액터에서 특수한 조건 처리가 필요할 경우 오버라이드하여 사용
	UFUNCTION(BlueprintNativeEvent)
	bool EvaluateInteractability() const;
	virtual bool EvaluateInteractability_Implementation() const;
	
	UFUNCTION(BlueprintNativeEvent)
	ECPInteractionDisplayState EvaluateDisplayState() const;
	virtual ECPInteractionDisplayState EvaluateDisplayState_Implementation() const;
	
	UFUNCTION(BlueprintNativeEvent)
	FName EvaluateTargetName() const;
	virtual FName EvaluateTargetName_Implementation() const;
	
	UFUNCTION(BlueprintNativeEvent)
	FText EvaluateInteractionPrompt() const;
	virtual FText EvaluateInteractionPrompt_Implementation() const;
	
	UFUNCTION(BlueprintNativeEvent)
	float EvaluateInteractionDuration() const;
	virtual float EvaluateInteractionDuration_Implementation() const;
	
	// --- Setter 함수 ---
	
	// 활성화/비활성화/숨김 여부 Enum값 설정
	// Enabled: 활성화(UI, 하이라이트 보임), Disabled: 비활성화(회색 UI만 보임), Hidden: 모두 숨김
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetDisplayState(ECPInteractionDisplayState NewDisplayState);
	
	// 상호작용 가능 여부 설정
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractability(bool bValue);
	
	// 상호작용 프롬프트 설정
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionPrompt(FText NewPrompt);
	
	// 상호작용 대상 액터 이름 설정
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetTargetName(FName NewName);
	
	// 시간형 상호작용 소요시간 설정
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionDuration(float NewDuration);

protected:
	// 상호작용 액터 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UCPInteractableComponent> InteractableComponent;
	
	// 상호작용 위젯 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TObjectPtr<UWidgetComponent> InteractionWidget;
};
