#pragma once

#include "CoreMinimal.h"
#include "Components/CPInteractableComponent.h"
#include "CPTutorialComponent.generated.h"

class UCPNPCDialogueWidget;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPTutorialComponent : public UCPInteractableComponent
{
	GENERATED_BODY()

public:
	UCPTutorialComponent();

	virtual bool CanInteract() const override;
	virtual bool TryInteract(AActor* Interactor) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	FName AssociatedQuestID;

	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	bool bIsCompletionActor = false;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPNPCDialogueWidget> DialogueWidgetClass;
};
