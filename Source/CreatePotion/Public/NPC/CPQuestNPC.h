#pragma once

#include "CoreMinimal.h"
#include "NPC/CPBaseNPC.h"
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

	void HandleLocationFadeFinished();


private:
	TArray<FText> FindDialogueLines(AActor* Interactor);

	// 결과 UI 표시 (대화형 퀘스트 완료 후)
	void ShowQuestResultWidget(FName QuestID);

protected:
	UPROPERTY(Transient)
	TObjectPtr<const UCPQuestNPCDataAsset> QuestData;

	UPROPERTY()
	TObjectPtr<UCPNPCDialogueWidget> ActiveDialogueWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UCPNPCComponent> InteractableComponent;

	FName CurrentQuestId = NAME_None;

	// 대화창이 닫힐 때 결과 UI를 띄울 퀘스트
	FName PendingResultQuestId = NAME_None;

	// 이동 페이드가 끝난 뒤 결과 UI를 띄울 퀘스트
	FName ResultAfterFadeQuestId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Interaction")
	float StopRadius = 80.f;

	bool bIsFocused = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Marker")
	TObjectPtr<UWidgetComponent> QuestMarkerWidget;

	UPROPERTY(Transient)
	FVector InitialQuestMarkerLocation = FVector::ZeroVector;
};