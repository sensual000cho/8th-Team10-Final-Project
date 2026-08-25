// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "QuestSettings.generated.h"

class UDataTable;

/*
	사용법 관련
	const UQuestSettings* QuestSettings = GetDefault<UQuestSettings>();
	if (!QuestSettings) return;
	UDataTable* Table = QuestSettings->QuestScriptTable.LoadSynchronous();
	if (!Table) return;
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "CreatePotion Quest Settings"))
class CREATEPOTION_API UQuestSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Quest
	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Quest")
	TSoftObjectPtr<UDataTable> QuestScriptTable;

	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Quest")
	TSoftObjectPtr<UDataTable> QuestAnswerTable;

	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Quest")
	TSoftObjectPtr<UDataTable> RandomQuestAnswerTable;
};