// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Quest/QuestTypes.h"
#include "CPStoryActor.generated.h"

class UCPNPCDialogueWidget;
class UCPInteractableComponent;

UCLASS()
class CREATEPOTION_API ACPStoryActor : public AActor
{
	GENERATED_BODY()

public:
	ACPStoryActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleInteract(AActor* Interactor);

public:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> DefaultRoot;

	UPROPERTY(EditDefaultsOnly, Category = "Lore")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Category = "Lore")
	TArray<FCPDialogueLine> LoreLines;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPNPCDialogueWidget> DialogueWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UCPInteractableComponent> InteractableComponent;
};