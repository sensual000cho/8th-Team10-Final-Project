// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "CPTutorialActor.generated.h"

class UCPNPCDialogueWidget;
class UCPInteractableComponent;
class UCPInventoryComponent;

UCLASS()
class CREATEPOTION_API ACPTutorialActor : public AActor
{
	GENERATED_BODY()

public:
	ACPTutorialActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleInteract(AActor* Interactor);

	void RefreshInteractability();

public:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> DefaultRoot;

	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	FName AssociatedQuestID;

	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	bool bIsCompletionActor = false;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPNPCDialogueWidget> DialogueWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UCPInteractableComponent> InteractableComponent;

private:
	FTimerHandle InteractionRefreshTimerHandle;
};