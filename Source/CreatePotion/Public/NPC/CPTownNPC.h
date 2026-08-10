#pragma once

#include "CoreMinimal.h"
#include "NPC/CPBaseNPC.h"
#include "CPTownNPC.generated.h"

class UCPNPCDialogueWidget;

UCLASS()
class CREATEPOTION_API ACPTownNPC : public ACPBaseNPC
{
	GENERATED_BODY()

public:
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UCPNPCDialogueWidget> DialogueWidgetClass;
};
