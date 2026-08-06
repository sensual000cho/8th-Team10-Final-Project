// CPTimedInteractable.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CPTimedInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCPTimedInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CREATEPOTION_API ICPTimedInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	float GetInteractionDuration(AActor* Interactor); // 채집, 조사시간 가져오기

	UFUNCTION(BlueprintNativeEvent)
	void OnInteractionStarted(AActor* Interactor); // 상호작용 시작

	// 현재는 사용하지 않음, 추후 취소 유무 및 조건이 확립되면 사용 예정
	UFUNCTION(BlueprintNativeEvent)
	void OnInteractionCancelled(AActor* Interactor); // 상호작용이 취소 됐을 때
};
