#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Resource/CPResourceType.h"
#include "CPSaveGame.generated.h"

// 저장되어야 할 데이터들 모음
UCLASS()
class CREATEPOTION_API UCPSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	// 채집물 상태 정보
	UPROPERTY()
	TMap<FCPResourceNodeKey, FCPResourceNodeState> NodeStates;
};
