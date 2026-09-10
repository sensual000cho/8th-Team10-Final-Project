// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPBaseInteractableActor.generated.h"

class UCPInteractableComponent;
class UWidgetComponent;

UCLASS()
class CREATEPOTION_API ACPBaseInteractableActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACPBaseInteractableActor();

	virtual void PostInitializeComponents() override;
protected:
	// Called when the game starts or when spawned
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- 상호작용 오버라이드 함수 ---
	// 실제 상호작용 실행
	UFUNCTION()
	virtual void HandleInteract(AActor* Interactor);

	// 시간형 상호작용 실행 (ex: 애니메이션 몽타주 재생 등)
	UFUNCTION()
	virtual void HandleInteractionStarted(AActor* Interactor);
	
protected:
	// 상호작용 액터 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UCPInteractableComponent> InteractableComponent;
	
	// 상호작용 위젯 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TObjectPtr<UWidgetComponent> InteractionWidget;
};
