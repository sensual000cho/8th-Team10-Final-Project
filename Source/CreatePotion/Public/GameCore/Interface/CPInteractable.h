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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnInteract(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionPrompt(); // "채집하기", "대화하기", "제조대 사용" 등 UI 텍스트

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FName GetInteractionName(); // 액터 상호작용 시 표시할 이름
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AActor* Interactor); // 조건 체크 (레벨, 아이템 소지 등)

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool ShouldShowUnavailableInteraction(AActor* Interactor);
};
