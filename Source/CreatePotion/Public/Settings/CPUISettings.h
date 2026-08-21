// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CPUISettings.generated.h"

/*
	사용법 관련
	const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
	if (!UISettings) return;

	TSubclassOf<UUserWidget> WidgetClass = UISettings->Class 이름.LoadSynchronous();
	if(!Widget Class) Return;
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="CreatePotion UI Settings"))
class CREATEPOTION_API UCPUISettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	// Quest
	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Quest")
	TSoftClassPtr<UUserWidget> QuestJournalWidgetClass;
	
	// Codex
	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Codex")
	TSoftClassPtr<UUserWidget> ForageableCodexWidgetClass;
	
	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Codex")
	TSoftClassPtr<UUserWidget> NPCCodexWidgetClass;
	
	// Interact
	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Interact")
	TSoftClassPtr<UUserWidget> InteractionWidgetClass;
	
	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Interact")
	TSoftClassPtr<UUserWidget> InteractionProgressWidgetClass;
	
	// Common
	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Common")
	TSoftClassPtr<UUserWidget> GoalWidgetClass;
	
	// Dialogue
	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Dialogue")
	TSoftClassPtr<UUserWidget> NPCDialogueWidgetClass;
	
	// Inventory
	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Inventory")
	TSoftClassPtr<UUserWidget> InventoryMainWidgetClass;
};
