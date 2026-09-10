#pragma once

#include "CoreMinimal.h"
#include "NPC/CPQuestNPC.h"
#include "CPTownNPC.generated.h"

class UCPNPCDialogueWidget;

UCLASS()
class CREATEPOTION_API ACPTownNPC : public ACPQuestNPC
{
	GENERATED_BODY()

public:
	virtual void HandleInteract(AActor* Interactor) override;
	virtual bool CanInteract(AActor* Interactor) const override;

protected:
	virtual const TArray<FName>& GetPotionQuestIDs() const override;

	virtual void OnDialogueClosed() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UCPNPCDialogueWidget> DialogueWidgetClass;

};