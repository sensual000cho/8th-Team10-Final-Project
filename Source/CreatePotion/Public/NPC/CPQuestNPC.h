#pragma once

#include "CoreMinimal.h"
#include "NPC/CPBaseNPC.h"
#include "CPQuestNPC.generated.h"

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
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual FText GetInteractionPrompt() const override;
	virtual void HandleInteract(AActor* Interactor) override;

	virtual void Tick(float DeltaTime) override;

	void HandleThrownPotionImpact(const TArray<FGameplayTag>& PotionEffectTags);

	UFUNCTION()
	virtual void ShowResultDialogue();

	virtual void UpdateOverheadWidgetsScale(float ScaleMultiplier) override;

protected:
	virtual void BeginPlay() override;
	virtual const TArray<FName>& GetPotionQuestIDs() const;

	virtual void RefreshInteractionState() override;

	FName FindAcceptedQuestId() const;
	virtual void OnTraceFocusLost();
	virtual void OnDialogueClosed();
	void UpdateQuestMarker();

	UFUNCTION()
	void HandleQuestUpdatedForMarker(FName QuestID, EQuestState NewState);

private:
	void UpdateGazeTracking();

protected:
	UPROPERTY(Transient)
	TObjectPtr<const UCPQuestNPCDataAsset> QuestData;

	UPROPERTY()
	TObjectPtr<UCPNPCDialogueWidget> ActiveDialogueWidget;

	FName CurrentQuestId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Interaction")
	float StopRadius = 80.0f;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Interaction")
	bool bIsInteracting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UWidgetComponent> InteractionWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Marker")
	TObjectPtr<UWidgetComponent> QuestMarkerWidget;

	UPROPERTY(Transient)
	FVector InitialQuestMarkerLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector InitialInteractionWidgetLocation = FVector::ZeroVector;

	bool bIsPlayerLooking = false;

private:
	FTimerHandle TraceLostTimerHandle;
};