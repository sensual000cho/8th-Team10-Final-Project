#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPNPCDialogueWidget.generated.h"

class UTextBlock;
class UHorizontalBox;
class UCPNPCDialogueButtonWidget;
class UCPTagSelectionWidget;

UCLASS()
class CREATEPOTION_API UCPNPCDialogueWidget : public UCPBasePopupWidget {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void InitDialogue(bool bIsWorkshopQuest, FName InQuestID, const FText& InNPCName, const FText& InDialogueText, class ACPLabNPC* InSourceLabNPC = nullptr);

protected:
    virtual void BindEvents() override;
    virtual void UnbindEvents() override;

private:
    UFUNCTION()
    void OnChoiceSelected(const FString& ButtonText);

    void PlayTypewriterEffect(const FText& InDialogueText);
    void OnTypewriterTick();
    void CreateChoiceButtons();

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_NPCName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_Dialogue;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> HBox_ChoiceList;

    UPROPERTY(EditDefaultsOnly, Category = "Dialogue")
    TSubclassOf<UCPNPCDialogueButtonWidget> DialogueButtonClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UCPTagSelectionWidget> TagSelectionWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Typewriter")
    float TypewriterSpeed = 0.05f;

private:
    FName CurrentQuestID;

    FTimerHandle TypewriterTimerHandle;
    FString FullDialogueText;
    FString CurrentDialogueText;
    int32 CurrentCharIndex;

    int32 CurrentHintLevel = 0;

    bool bCurrentIsWorkshopQuest = false;

    TWeakObjectPtr<class ACPLabNPC> SourceLabNPC;

};