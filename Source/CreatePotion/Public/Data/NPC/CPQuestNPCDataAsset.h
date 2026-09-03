#pragma once

#include "CoreMinimal.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "CPQuestNPCDataAsset.generated.h"

UCLASS(BlueprintType)
class CREATEPOTION_API UCPQuestNPCDataAsset : public UCPNPCDataAsset
{
	GENERATED_BODY()

public:
	// [마을 전용] 부여할 마을 퀘스트 ID 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Quest")
	TArray<FName> TownQuestIDs;

	// [공방 전용] 담당할 공방 퀘스트 ID 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Quest")
	TArray<FName> LabQuestIDs;
};