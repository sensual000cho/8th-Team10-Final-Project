#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueButtonWidget.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/QuestManager.h"
#include "TimerManager.h"
#include "UI/Widgets/Lab/TagChoice/CPTagSelectionWidget.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameState/CPLabGameState.h" 
#include "Lab/Component/CPLabPotionSessionComponent.h"
#include "NPC/CPLabNPC.h"

void UCPNPCDialogueWidget::InitDialogue(bool bIsWorkshopQuest, FName InQuestID, const FText& InNPCName, const FText& InDialogueText, ACPLabNPC* InSourceLabNPC)
{
    CurrentQuestID = InQuestID;
    bCurrentIsWorkshopQuest = bIsWorkshopQuest;
    SourceLabNPC = InSourceLabNPC;
    bIsPotionResultDialogue = false;

    // 기본적으로 NPC가 넘겨준 텍스트를 사용하도록 설정
    FText TextToPlay = InDialogueText;

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>())
        {
            // 현재 퀘스트의 힌트 레벨을 가져옴
            CurrentHintLevel = QuestManager->GetQuestHintLevel(CurrentQuestID);

            // 공방 퀘스트라면 현재 힌트 레벨에 맞춰 대사를 덮어씌움
            if (bCurrentIsWorkshopQuest)
            {
                if (CurrentHintLevel == 1)
                {
                    TextToPlay = QuestManager->GetSessionHintTextDetailed(CurrentQuestID);
                }
                else if (CurrentHintLevel == 2)
                {
                    TextToPlay = QuestManager->GetSessionHintTextDetailed2(CurrentQuestID);
                }
            }
        }
    }

    if (Text_NPCName) {
        Text_NPCName->SetText(InNPCName);
    }

    PlayTypewriterEffect(TextToPlay);
}

void UCPNPCDialogueWidget::InitResultDialogue(bool bIsWorkshopQuest, FName InQuestID, const FText& InNPCName, const FText& InDialogueText, ACPLabNPC* InSourceLabNPC)
{
    CurrentQuestID = InQuestID;
    bCurrentIsWorkshopQuest = bIsWorkshopQuest;
    SourceLabNPC = InSourceLabNPC;
    bIsPotionResultDialogue = true; 

    if (Text_NPCName) {
        Text_NPCName->SetText(InNPCName);
    }

    PlayTypewriterEffect(InDialogueText);
}

void UCPNPCDialogueWidget::BindEvents()
{
    Super::BindEvents();
}

void UCPNPCDialogueWidget::UnbindEvents()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(TypewriterTimerHandle);
    }
    Super::UnbindEvents();
}

void UCPNPCDialogueWidget::PlayTypewriterEffect(const FText& InDialogueText)
{
    if (HBox_ChoiceList) {
        HBox_ChoiceList->ClearChildren();
    }

    FullDialogueText = InDialogueText.ToString();
    CurrentDialogueText.Empty();
    CurrentCharIndex = 0;

    if (Text_Dialogue) {
        Text_Dialogue->SetText(FText::GetEmpty());
    }

    GetWorld()->GetTimerManager().ClearTimer(TypewriterTimerHandle);

    GetWorld()->GetTimerManager().SetTimer(
        TypewriterTimerHandle,
        this,
        &UCPNPCDialogueWidget::OnTypewriterTick,
        TypewriterSpeed,
        true
    );
}

void UCPNPCDialogueWidget::OnTypewriterTick()
{
    if (CurrentCharIndex < FullDialogueText.Len())
    {
        CurrentDialogueText.AppendChar(FullDialogueText[CurrentCharIndex]);
        CurrentCharIndex++;

        if (Text_Dialogue) {
            Text_Dialogue->SetText(FText::FromString(CurrentDialogueText));
        }
    }
    else
    {
        GetWorld()->GetTimerManager().ClearTimer(TypewriterTimerHandle);
        CreateChoiceButtons();
    }
}

void UCPNPCDialogueWidget::CreateChoiceButtons()
{
    if (!HBox_ChoiceList || !DialogueButtonClass) { return; }

    HBox_ChoiceList->ClearChildren();

    //결과 보기 버튼
    if (bIsPotionResultDialogue)
    {
        UCPNPCDialogueButtonWidget* NewButton = CreateWidget<UCPNPCDialogueButtonWidget>(this, DialogueButtonClass);
        if (NewButton) {
            NewButton->SetButtonText(FText::FromString(TEXT("결과 보기")));
            NewButton->OnButtonClickedEvent.AddDynamic(this, &UCPNPCDialogueWidget::OnChoiceSelected);
            HBox_ChoiceList->AddChild(NewButton);
        }
        return;
    }

    if (!bCurrentIsWorkshopQuest) {
        // [마을 퀘스트] "네" 버튼 생성
        UCPNPCDialogueButtonWidget* NewButton = CreateWidget<UCPNPCDialogueButtonWidget>(this, DialogueButtonClass);
        if (NewButton) {
            NewButton->SetButtonText(FText::FromString(TEXT("네")));
            NewButton->OnButtonClickedEvent.AddDynamic(this, &UCPNPCDialogueWidget::OnChoiceSelected);
            HBox_ChoiceList->AddChild(NewButton);
        }
    }
    else {
        TArray<FString> Choices;

        if (CurrentHintLevel < 2) {
            // 아직 더 볼 힌트가 남아있다면 두 버튼 모두 표시
            Choices = { TEXT("알겠습니다"), TEXT("네? 그게 뭐죠?") };
        }
        else {
            // 마지막 2차 힌트까지 다 봤다면 "알겠습니다" 버튼만 표시
            Choices = { TEXT("알겠습니다") };
        }

        for (const FString& ChoiceText : Choices) {
            UCPNPCDialogueButtonWidget* NewButton = CreateWidget<UCPNPCDialogueButtonWidget>(this, DialogueButtonClass);
            if (NewButton) {
                NewButton->SetButtonText(FText::FromString(ChoiceText));
                NewButton->OnButtonClickedEvent.AddDynamic(this, &UCPNPCDialogueWidget::OnChoiceSelected);
                HBox_ChoiceList->AddChild(NewButton);
            }
        }
    }
}
void UCPNPCDialogueWidget::OnChoiceSelected(const FString& ButtonText) {
    UGameInstance* GI = GetGameInstance();
    if (!GI) { return; }

    UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>();
    UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>();

    if (ButtonText == TEXT("결과 보기"))
    {
        if (ACPLabNPC* LabNPC = SourceLabNPC.Get())
        {
            // NPC에게 명령해서 ResultWidget을 열도록 함
            LabNPC->OpenResultWidget();
        }
        RequestClose(); // 대화창 닫기
        return;
    }
    if (ButtonText == TEXT("네")) {
        if (QuestManager && !CurrentQuestID.IsNone()) {
            QuestManager->AcceptQuest(CurrentQuestID);
        }
        RequestClose();
    }
    else if (ButtonText == TEXT("알겠습니다")) {
      
        if (ACPLabNPC* LabNPC = SourceLabNPC.Get())
        {
            LabNPC->SetRequestConfirmed(true);
        }

        if (UIManager && TagSelectionWidgetClass) {
            UUserWidget* CreatedWidget = UIManager->PushWidget(TagSelectionWidgetClass);

            if (UCPTagSelectionWidget* TagSelectionPopup = Cast<UCPTagSelectionWidget>(CreatedWidget)) {
                TagSelectionPopup->InitTagSelectionWidget(CurrentQuestID);
            }
        }

        RequestClose();
    }
    else if (ButtonText == TEXT("네? 그게 뭐죠?")) {
        if (QuestManager && !CurrentQuestID.IsNone()) {
            CurrentHintLevel++;
            QuestManager->SetQuestHintLevel(CurrentQuestID, CurrentHintLevel);

            FText NextHint;
            if (CurrentHintLevel == 1) {
                NextHint = QuestManager->GetSessionHintTextDetailed(CurrentQuestID);
            }
            else if (CurrentHintLevel == 2) {
                NextHint = QuestManager->GetSessionHintTextDetailed2(CurrentQuestID);
            }

            PlayTypewriterEffect(NextHint);
        }
    }
}