// CPPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "CPPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UCPItemContainerComponent;
class UCPContainerMainWidget;

UCLASS()
class CREATEPOTION_API ACPPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Test")
	void ToggleLabUI();
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UCPContainerMainWidget> LabUIClass;

	UPROPERTY()
	UCPContainerMainWidget* LabUIInstance;


#pragma region UI
public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OnQuestTogglePressed();
	
protected:
	UPROPERTY(EditAnywhere, Category = "UI|Tags")
	FGameplayTag QuestToggleTag;
	
	
#pragma endregion
};
