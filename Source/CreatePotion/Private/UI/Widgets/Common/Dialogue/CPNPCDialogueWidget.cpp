#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueButtonWidget.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/QuestManager.h"
#include "TimerManager.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameState/CPLabGameState.h" 
#include "Lab/Component/CPLabPotionSessionComponent.h"
#include "NPC/CPLabNPC.h"
#include "GameMode/CPLabGameMode.h"

void UCPNPCDialogueWidget::InitResultDialogue(bool bIsWorkshopQuest, FName InQuestID, const FText& InNPCName, const FText& InDialogueText, ACPLabNPC* InSourceLabNPC)
{
    // [신규] 이전 대화의 여러 줄 상태가 남아있지 않도록 초기화
    DialogueLines.Empty();
    CurrentLineIndex = 0;

    CurrentQuestID = InQuestID;
    bCurrentIsWorkshopQuest = bIsWorkshopQuest;
    SourceLabNPC = InSourceLabNPC;
    bIsPotionResultDialogue = true; 

    if (Text_NPCName) {
        Text_NPCName->SetText(InNPCName);
    }

    PlayTypewriterEffect(InDialogueText);
}

// [신규] 여러 줄 대사(TArray<FText>)를 받는 오버로드.
// DialogueLines에 배열을 저장하고, 0번째 줄부터 재생 시작.
void UCPNPCDialogueWidget::InitDialogueLines(bool bIsWorkshopQuest, FName InQuestID, const FText& InNPCName, const TArray<FText>& InDialogueLines, ACPLabNPC* InSourceLabNPC)
{
    CurrentQuestID = InQuestID;
    bCurrentIsWorkshopQuest = bIsWorkshopQuest;
    SourceLabNPC = InSourceLabNPC;
    bIsPotionResultDialogue = false;

    DialogueLines = InDialogueLines;
    CurrentLineIndex = 0;

    if (Text_NPCName) {
        Text_NPCName->SetText(InNPCName);
    }

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>())
        {
            CurrentHintLevel =
                QuestManager->GetQuestHintLevel(CurrentQuestID);

            if (bCurrentIsWorkshopQuest)
            {
                DialogueLines.Empty();

                DialogueLines.Add(
                    QuestManager->GetCurrentSessionHintText(CurrentQuestID)
                );
            }
            else
            {
                DialogueLines = InDialogueLines;
            }
        }
    }

    PlayCurrentLine();
}

// [신규] DialogueLines[CurrentLineIndex]를 꺼내 타자기 효과로 재생.
// "다음" 버튼 클릭 시에도 이 함수가 다시 호출되어 다음 줄을 재생함.
void UCPNPCDialogueWidget::PlayCurrentLine()
{
    if (DialogueLines.IsValidIndex(CurrentLineIndex))
    {
        PlayTypewriterEffect(DialogueLines[CurrentLineIndex]);
    }
}

void UCPNPCDialogueWidget::BindEvents()
{
    Super::BindEvents();
    SetIsFocusable(true);

    if (Button_SkipAll) {
        Button_SkipAll->OnClicked.AddUniqueDynamic(this, &UCPNPCDialogueWidget::OnSkipAllClicked);
    }
}

void UCPNPCDialogueWidget::UnbindEvents()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(TypewriterTimerHandle);
    }
    if (Button_SkipAll) {
        Button_SkipAll->OnClicked.RemoveDynamic(this, &UCPNPCDialogueWidget::OnSkipAllClicked);
    }

    Super::UnbindEvents();
}
void UCPNPCDialogueWidget::OnChoiceSelected(const FString& ButtonText) {

    // [신규] "다음" 버튼 - 다음 줄로 넘어가며 재생. 이 처리는 QuestManager 등이 필요 없어 가장 먼저 분기.
    if (ButtonText == TEXT("다음"))
    {
        CurrentLineIndex++;
        PlayCurrentLine();
        return;
    }

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
        if (UWorld* World = GetWorld())
        {
            if (ACPLabGameMode* LabGameMode = World->GetAuthGameMode<ACPLabGameMode>())
            {
                if (UCPLabPotionSessionComponent* Session = LabGameMode->GetPotionSession())
                {
                    FCPLabPotionRequestState ActiveRequestState;
                    if (Session->GetActiveRequestState(ActiveRequestState))
                    {
                        if (ActiveRequestState.Phase == ECPLabPotionRequestPhase::Selected)
                        {
                            LabGameMode->AdvancePotionRequest();
                            UE_LOG(LogTemp, Warning, TEXT("[CPNPCDialogueWidget] 포션 세션 상태 Selected-> Processing"));
                        }
                    }
                }
            }
        }
        RequestClose();
    }

    else if (ButtonText == TEXT("네? 그게 뭐죠?")) {
        if (QuestManager && !CurrentQuestID.IsNone()) {
            CurrentHintLevel++;
            QuestManager->SetQuestHintLevel(CurrentQuestID, CurrentHintLevel);
            FText NextHint = QuestManager->GetCurrentSessionHintText(CurrentQuestID);

            PlayTypewriterEffect(NextHint);
        }
    }
}
void UCPNPCDialogueWidget::PlayTypewriterEffect(const FText& InDialogueText)
{
    if (HBox_ChoiceList) {
        HBox_ChoiceList->ClearChildren();
    }
    if (Button_SkipAll) {
        if (DialogueLines.Num() == 0 || CurrentLineIndex >= DialogueLines.Num() - 1) {
            Button_SkipAll->SetVisibility(ESlateVisibility::Collapsed);
        }
        else {
            Button_SkipAll->SetVisibility(ESlateVisibility::Visible);
        }
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
FReply UCPNPCDialogueWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        if (GetWorld()->GetTimerManager().IsTimerActive(TypewriterTimerHandle))
        {
            SkipTypewriterEffect();
        }
        else if (DialogueLines.Num() > 0 && CurrentLineIndex < DialogueLines.Num() - 1)
        {
            OnChoiceSelected(TEXT("다음"));
        }

        return FReply::Handled();
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UCPNPCDialogueWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::SpaceBar)
    {
        if (GetWorld()->GetTimerManager().IsTimerActive(TypewriterTimerHandle))
        {
            SkipTypewriterEffect();
        }
        else if (DialogueLines.Num() > 0 && CurrentLineIndex < DialogueLines.Num() - 1)
        {
            OnChoiceSelected(TEXT("다음"));
        }

        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
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

        // [신규] 대사 중 다음 줄이 남아있으면 "다음" 버튼만 표시.
        // 마지막 줄이거나 단일 텍스트 대화(DialogueLines 비어있음)면 기존 선택지 버튼 표시.
        if (DialogueLines.Num() > 0 && CurrentLineIndex < DialogueLines.Num() - 1)
        {
            CreateContinueButton();
        }
        else
        {
            CreateChoiceButtons();
        }
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

// [신규] 여러 줄 대사 중간에 표시할 "다음" 버튼 하나만 생성.
// 기존 DialogueButtonClass(다른 선택지 버튼과 동일한 클래스)를 재사용함.
void UCPNPCDialogueWidget::CreateContinueButton()
{
    if (!HBox_ChoiceList || !DialogueButtonClass) { return; }

    HBox_ChoiceList->ClearChildren();

    UCPNPCDialogueButtonWidget* NewButton = CreateWidget<UCPNPCDialogueButtonWidget>(this, DialogueButtonClass);
    if (NewButton) {
        NewButton->SetButtonText(FText::FromString(TEXT("다음")));
        NewButton->OnButtonClickedEvent.AddDynamic(this, &UCPNPCDialogueWidget::OnChoiceSelected);
        HBox_ChoiceList->AddChild(NewButton);
    }
}

void UCPNPCDialogueWidget::OnSkipAllClicked()
{
    if (DialogueLines.Num() > 0)
    {
        CurrentLineIndex = DialogueLines.Num() - 1;
        FullDialogueText = DialogueLines[CurrentLineIndex].ToString();
    }

    GetWorld()->GetTimerManager().ClearTimer(TypewriterTimerHandle);
    CurrentCharIndex = FullDialogueText.Len();

    if (Text_Dialogue) {
        Text_Dialogue->SetText(FText::FromString(FullDialogueText));
    }
    CreateChoiceButtons();

    if (Button_SkipAll) {
        Button_SkipAll->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UCPNPCDialogueWidget::SkipTypewriterEffect()
{
    if (!GetWorld()->GetTimerManager().IsTimerActive(TypewriterTimerHandle))
    {
        return;
    }

    GetWorld()->GetTimerManager().ClearTimer(TypewriterTimerHandle);

    CurrentCharIndex = FullDialogueText.Len();
    if (Text_Dialogue) {
        Text_Dialogue->SetText(FText::FromString(FullDialogueText));
    }
    if (DialogueLines.Num() > 0 && CurrentLineIndex < DialogueLines.Num() - 1)
    {
        CreateContinueButton();
    }
    else
    {
        CreateChoiceButtons();
    }
}
