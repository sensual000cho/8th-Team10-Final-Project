#pragma once

#include "CoreMinimal.h"
#include "NPC/CPBaseNPC.h"
#include "CPAnimalNPC.generated.h"

class UQuestManager;

UCLASS()
class CREATEPOTION_API ACPAnimalNPC : public ACPBaseNPC
{
	GENERATED_BODY()

public:
	ACPAnimalNPC();

	virtual FName GetPotionNPCId() const override;

	void HandleThrownPotionImpact(const TArray<FGameplayTag>& PotionEffectTags);

	void ShowResultWidget(FName QuestID);
	
protected:
	UPROPERTY(Transient)
	mutable FName CachedPotionNPCId = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Quest")
	FName AssociatedQuestID;
};