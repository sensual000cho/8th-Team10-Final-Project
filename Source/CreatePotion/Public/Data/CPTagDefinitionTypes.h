// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CPTagDefinitionTypes.generated.h"

USTRUCT(BlueprintType)
struct FCPTagCombinationEntry
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag")
	FGameplayTag OtherTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag")
	FGameplayTag ResultTag;
};

USTRUCT(BlueprintType)
struct FCPTagDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag")
	FGameplayTag Tag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag")
	FText DisplayName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag")
	TArray<FCPTagCombinationEntry> Combinations;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FLinearColor LiquidColor01;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FLinearColor LiquidColor02;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FLinearColor SurfaceColor01;
};
