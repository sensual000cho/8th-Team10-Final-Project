// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "CPQuestBoard.generated.h"

class UCPInteractableComponent;

UCLASS()
class CREATEPOTION_API ACPQuestBoard : public AActor
{
	GENERATED_BODY()

public:
	ACPQuestBoard();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleInteract(AActor* Interactor);

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> QuestBoardWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UCPInteractableComponent> InteractableComponent;
};