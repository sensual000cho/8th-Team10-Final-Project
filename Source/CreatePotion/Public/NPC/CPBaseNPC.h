#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameCore/Interface/CPInteractable.h"
#include "NPC/CPNPCTypes.h"
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

	// 나중에 할 StateTree용 현재 상황과 감정에 맞는 대사 및 애니메이션 몽타주 데이터 반환
	UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
	bool GetDialogueEntryForCurrentState(FCPNPCDialogueEntry& OutEntry) const;

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

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|State")
	ECPNPCEmotion CurrentEmotion = ECPNPCEmotion::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|State")
	ECPNPCSituation CurrentSituation = ECPNPCSituation::Greeting; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|State")
	ECPNPCActivityState CurrentActivityState = ECPNPCActivityState::Idle; 

};
