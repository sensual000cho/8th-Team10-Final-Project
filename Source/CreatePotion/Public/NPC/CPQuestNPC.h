#pragma once

#include "CoreMinimal.h"
#include "NPC/CPBaseNPC.h"
#include "Quest/QuestTypes.h"
#include "CPQuestNPC.generated.h"

class UCPNPCComponent;
class UCPNPCDialogueWidget;
class UCPQuestNPCDataAsset;
class UWidgetComponent;

UCLASS()
class CREATEPOTION_API ACPQuestNPC : public ACPBaseNPC
{
	GENERATED_BODY()

public:
	ACPQuestNPC();
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;
	
	virtual void StartDialogue(AActor* Interactor);
	
	void SetFocused(bool bFocused);

	void HandleThrownPotionImpact(const TArray<FGameplayTag>& PotionEffectTags);

	UFUNCTION()
	virtual void ShowResultDialogue();

	virtual void UpdateOverheadWidgetsScale(float ScaleMultiplier) override;

protected:
	virtual void BeginPlay() override;

	FName FindAcceptedQuestId() const;
	
	virtual void OnDialogueClosed();
	void UpdateQuestMarker();

	UFUNCTION()
	void HandleQuestUpdatedForMarker(FName QuestID, EQuestState NewState);
	
private:
	TArray<FCPDialogueLine> FindDialogueLines(AActor* Interactor);

protected:
	UPROPERTY(Transient)
	TObjectPtr<const UCPQuestNPCDataAsset> QuestData;

	UPROPERTY()
	TObjectPtr<UCPNPCDialogueWidget> ActiveDialogueWidget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UCPNPCComponent> InteractableComponent;

	FName CurrentQuestId = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Interaction")
	float StopRadius = 80.f;
	
	bool bIsFocused = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Marker")
	TObjectPtr<UWidgetComponent> QuestMarkerWidget;

	UPROPERTY(Transient)
	FVector InitialQuestMarkerLocation = FVector::ZeroVector;
};