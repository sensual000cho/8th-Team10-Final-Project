// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CPDTSettings.generated.h"

class UDataTable;
/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="CreatePotion Data Table Settings"))
class CREATEPOTION_API UCPDTSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Config, BlueprintReadWrite, Category = "Tag")
	TSoftObjectPtr<UDataTable> TagDefinitionTable;

	UPROPERTY(EditAnywhere, Config, BlueprintReadWrite, Category = "Quest")
	TSoftObjectPtr<UDataTable> QuestScriptTable;
	
	UPROPERTY(EditAnywhere, Config, BlueprintReadWrite, Category = "Quest")
	TSoftObjectPtr<UDataTable> QuestAnswerTable;
	
	UPROPERTY(EditAnywhere, Config, BlueprintReadWrite, Category = "Quest")
	TSoftObjectPtr<UDataTable> RandomQuestAnswerTable;
};
