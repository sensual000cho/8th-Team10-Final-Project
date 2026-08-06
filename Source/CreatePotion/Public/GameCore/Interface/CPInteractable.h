// CPInteractable.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CPInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCPInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CREATEPOTION_API ICPInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void OnInteract(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent)
	FText GetInteractionPrompt(); // "채집하기", "대화하기", "제조대 사용" 등 UI 텍스트

	UFUNCTION(BlueprintNativeEvent)
	bool CanInteract(AActor* Interactor); // 조건 체크 (레벨, 아이템 소지 등)
};
