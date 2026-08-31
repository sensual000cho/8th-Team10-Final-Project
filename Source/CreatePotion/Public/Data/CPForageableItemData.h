// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "CPForageableItemData.generated.h"

class ACPThrowablePropBase;

UCLASS(BlueprintType)
class CREATEPOTION_API UCPForageableItemData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;
	
	// Tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tag")
	TArray<FGameplayTag> TagAxes;
	
	// 인벤토리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 MaxStack;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop|Forageable")
	TSoftObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop|Seed")
	TSoftObjectPtr<UTexture> Icon_seed;

	// 인벤토리 임시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 ContainerSizeX = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 ContainerSizeY = 1;
	
	// Prop	
	// AActor -> 관련 액터 구현 시 변경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop|Forageable")
	TSoftClassPtr<ACPThrowablePropBase> AlchemyPropClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prop|Seed")
	TSoftClassPtr<AActor> SeedPropClass;
	
	// 도감
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Codex")
	TArray<FName> CodexTextKeys;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Codex")
	TSoftObjectPtr<UTexture2D> CodexImage;
};
