#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TObjectPtr<UCPNPCDataAsset> NPCData;

};
