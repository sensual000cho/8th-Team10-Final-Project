// CPPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "CPPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UCPItemContainerComponent;

UCLASS()
class CREATEPOTION_API ACPPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;
	
	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;


public:
	UPROPERTY(BlueprintReadWrite, Category = "Container")
	UCPItemContainerComponent* CurrentInteractingContainer = nullptr;

#pragma region UI
public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OnQuestTogglePressed();
	
protected:
	UPROPERTY(EditAnywhere, Category = "UI|Tags")
	FGameplayTag QuestToggleTag;
	
	
#pragma endregion
};


};
