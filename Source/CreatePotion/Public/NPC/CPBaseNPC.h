#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameCore/Interface/CPInteractable.h"
#include "CPBaseNPC.generated.h"

class UCPNPCDataAsset;
class UQuestManager;

UCLASS()
class CREATEPOTION_API ACPBaseNPC : public ACharacter, public ICPInteractable
{
	GENERATED_BODY()

public:
	
	ACPBaseNPC();

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void InitializeFromDataAsset();

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void FitCapsuleToMesh(USkeletalMesh* NPCMesh);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TObjectPtr<UCPNPCDataAsset> NPCData;

};
