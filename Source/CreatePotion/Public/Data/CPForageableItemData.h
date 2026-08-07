// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "CPForageableItemData.generated.h"

USTRUCT(BlueprintType)
struct FAlchemyProperty
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag Tag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Value = 0;
};

UCLASS(BlueprintType)
class CREATEPOTION_API UCPForageableItemData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;
	
	// Tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tag")
	TArray<FAlchemyProperty> TagAxes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tag")
	FAlchemyProperty SpecialProperty;
	
	// 인벤토리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 MaxStack;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TSoftObjectPtr<UTexture2D> Icon;

	// 인벤토리 임시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 ContainerSizeX = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 ContainerSizeY = 1;
	
	// Prop	
	// AActor -> 관련 액터 구현 시 변경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop")
	TSoftClassPtr<AActor> AlchemyPropClass;
	
	// 도감
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Codex")
	TArray<FName> CodexTextKeys;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Codex")
	TSoftObjectPtr<UTexture2D> CodexImage;
};