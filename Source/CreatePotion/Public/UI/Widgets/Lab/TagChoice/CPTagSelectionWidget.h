#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "GameplayTagContainer.h"
#include "CPTagSelectionWidget.generated.h"

class UScrollBox;
class UTextBlock;
class UButton;
class UCPTagEntryWidget;
class UCPTagRangeWidget;
class UCPHintWidget;

UCLASS()
class CREATEPOTION_API UCPTagSelectionWidget : public UCPBasePopupWidget
{
	GENERATED_BODY()

public:
	void InitTagSelectionWidget(FName InQuestID, const TArray<FGameplayTag>& InExistingSelectedTags = {}, const TMap<FGameplayTag, int32>& InExistingSavedValues = {});

protected:
	virtual void BindEvents() override;
	virtual void UnbindEvents() override;

private:
	void PopulateTagList();
	void UpdateSelectedTagsUI();

	UFUNCTION() 
	void OnTagEntrySelected(FGameplayTag SelectedTag);

	UFUNCTION() 
	void OnSlot1Clicked();
	UFUNCTION() 
	void OnSlot2Clicked();
	UFUNCTION() 
	void OnSlot3Clicked();

	UFUNCTION() 
	void OnConfirmClicked();

private:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox_TagList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCPHintWidget> UI_HintWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Confirm;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Slot1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_Slot1;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Slot2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_Slot2;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Slot3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_Slot3;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPTagEntryWidget> TagEntryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPTagRangeWidget> TagRangeWidgetClass;

	FName CurrentQuestID;
	TArray<FGameplayTag> SelectedTags;
	int32 CurrentHintLevel = 0;

	TMap<FGameplayTag, int32> SavedTagValues;

	const int32 MaxSelectableTags = 3;
};