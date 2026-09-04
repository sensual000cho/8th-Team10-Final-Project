// CPItemInstanceTypes.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CPItemInstanceTypes.generated.h"

class UCPForageableItemData;

USTRUCT(BlueprintType)
struct FCPItemInstance
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TObjectPtr<UCPForageableItemData> SourceItemData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TArray<FGameplayTag> CurrentEffects;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	bool bIsSeed = false;

	bool IsValid() const { return SourceItemData != nullptr; }
};
