// CPPotionData.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "CPPotionData.generated.h"

class ACPPotionActor;

UCLASS(BlueprintType)
class CREATEPOTION_API UCPPotionData : public UDataAsset
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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TSoftObjectPtr<UTexture2D> Icon;

	// 인벤토리 임시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 ContainerSizeX = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 ContainerSizeY = 1;
	
	// Potion
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion")
	TSoftClassPtr<ACPPotionActor> PotionActorClass;
	
	// 도감
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Codex")
	TArray<FName> CodexTextKeys;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Codex")
	TSoftObjectPtr<UTexture2D> CodexImage;
};
