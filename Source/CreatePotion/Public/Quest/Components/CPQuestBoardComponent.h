#pragma once

#include "CoreMinimal.h"
#include "Components/CPInteractableComponent.h"
#include "CPQuestBoardComponent.generated.h"

class UUserWidget;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPQuestBoardComponent : public UCPInteractableComponent
{
	GENERATED_BODY()

public:
	UCPQuestBoardComponent();

	virtual bool TryInteract(AActor* Interactor) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> QuestBoardWidgetClass;
};
