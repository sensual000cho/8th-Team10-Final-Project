#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameCore/Interface/CPinteractable.h"
#include "CPBaseNPC.generated.h"

class UCPNPCDataAsset;

UCLASS()
class CREATEPOTION_API ACPBaseNPC : public ACharacter
{
	GENERATED_BODY()

public:
	
	ACPBaseNPC();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void InitializeFromDataAsset();

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void FitCapsuleToMesh(USkeletalMesh* NPCMesh);

	void OnInteract_Implementation(AActor* Interactor);
	FText GetInteractionPrompt_Implementation();
	bool CanInteract_Implementation(AActor* Interactor);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TObjectPtr<UCPNPCDataAsset> NPCData;

};
