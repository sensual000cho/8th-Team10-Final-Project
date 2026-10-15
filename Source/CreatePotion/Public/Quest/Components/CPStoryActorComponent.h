#pragma once

#include "CoreMinimal.h"
#include "Components/CPInteractableComponent.h"
#include "Quest/QuestTypes.h"
#include "CPStoryActorComponent.generated.h"

class UCPNPCDialogueWidget;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPStoryActorComponent : public UCPInteractableComponent
{
	GENERATED_BODY()

public:
	UCPStoryActorComponent();

	virtual bool TryInteract(AActor* Interactor) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Lore")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Category = "Lore")
	TArray<FCPDialogueLine> LoreLines;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPNPCDialogueWidget> DialogueWidgetClass;
};
