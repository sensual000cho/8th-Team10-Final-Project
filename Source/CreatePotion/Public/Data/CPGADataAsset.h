#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Abilities/GameplayAbility.h"
#include "CPGADataAsset.generated.h"

UCLASS(BlueprintType)
class CREATEPOTION_API UCPGADataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category="GA")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;
	
};
