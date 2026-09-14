// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Quest/QuestTypes.h"
#include "CPQuestMarkerWidget.generated.h"

class UTextBlock;

UCLASS()
class CREATEPOTION_API UCPQuestMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetMarkerState(EQuestMarkerState NewState);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MarkerText;
};
