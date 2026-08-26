// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameCore/Interface/CPInteractable.h"
#include "Blueprint/UserWidget.h"
#include "CPQuestBoard.generated.h"

class UUserWidget;

UCLASS()
class CREATEPOTION_API ACPQuestBoard : public AActor, public ICPInteractable
{
	GENERATED_BODY()

public:
	ACPQuestBoard();

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() override;

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> QuestBoardWidgetClass;
};
