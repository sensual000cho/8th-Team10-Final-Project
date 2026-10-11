#pragma once

#include "CoreMinimal.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "Quest/QuestTypes.h"
#include "CPQuestNPCDataAsset.generated.h"

UCLASS(BlueprintType)
class CREATEPOTION_API UCPQuestNPCDataAsset : public UCPNPCDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Quest")
	TArray<FName> QuestIDs;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Quest")
		TArray<FCPDialogueLine> CompletedDialogueLines;
	};