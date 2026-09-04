#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueButtonWidget.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "Quest/QuestManager.h"
#include "TimerManager.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "NPC/CPLabNPC.h"

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

                FText HintText = QuestManager->GetSessionHintText(CurrentQuestID);
                if (!HintText.IsEmpty())
                {
                    DialogueLines.Add(HintText);
                }

                TArray<FText> StoryLines = QuestManager->GetNPCStoryLines(CurrentQuestID);
                if (StoryLines.Num() > 0)
                {
                    DialogueLines.Append(StoryLines);
                }
            }
            else
            {
                DialogueLines = InDialogueLines;
            }
        }
    }

    PlayCurrentLine();
}

void UCPNPCDialogueWidget::InitResultDialogue(bool bIsWorkshopQuest, FName InQuestID, const FText& InNPCName,
    const FText& InDialogueText, class ACPLabNPC* InSourceLabNPC, AActor* InInteractor)
{
    // [신규] 이전 대화의 여러 줄 상태가 남아있지 않도록 초기화
    DialogueLines.Empty();
    CurrentLineIndex = 0;

    CurrentQuestID = InQuestID;
    bCurrentIsWorkshopQuest = bIsWorkshopQuest;
    SourceLabNPC = InSourceLabNPC;
    ResultInteractor = InInteractor;
    bIsPotionResultDialogue = true;

    if (Text_NPCName) {
        Text_NPCName->SetText(InNPCName);
    }

    PlayTypewriterEffect(InDialogueText);
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
            LabNPC->OpenResultWidget(ResultInteractor.Get());
        }
        RequestClose(); // 대화창 닫기
        return;
    }
    if (ButtonText == TEXT("네")) {
        if (QuestManager && !CurrentQuestID.IsNone()) {
            if (QuestManager->GetQuestState(CurrentQuestID) == EQuestState::NotAccepted) {
                QuestManager->AcceptQuest(CurrentQuestID);
            }
            else {
                UE_LOG(LogTemp, Log, TEXT("이미 수락되었거나 완료된 퀘스트"));
            }
        }
        RequestClose();
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
            // 신규 추가
            // 마지막 줄까지 도달한 시점. 대화형 퀘스트의 완료 확정 대사(NPCStoryLines)라면
            // 여기서 완료 처리. bCurrentIsWorkshopQuest가 false인 마을 NPC 흐름에서만 해당.
            UE_LOG(LogTemp, Warning, TEXT("[완료체크] bWorkshop=%d, bResult=%d, QuestID=%s"),
                bCurrentIsWorkshopQuest, bIsPotionResultDialogue, *CurrentQuestID.ToString());

            if (!bCurrentIsWorkshopQuest && !bIsPotionResultDialogue)
            {
                if (UGameInstance* GI = GetGameInstance())
                {
                    if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>())
                    {
                        EQuestState State = QuestManager->GetQuestState(CurrentQuestID);
                        EQuestCompletionType Type = QuestManager->GetQuestCompletionType(CurrentQuestID);

                        UE_LOG(LogTemp, Warning, TEXT("[완료체크] State=%d, CompletionType=%d"),
                            (int32)State, (int32)Type);

                        if (State == EQuestState::Accepted
                            && Type == EQuestCompletionType::Dialogue)
                        {
                            bool bSuccess = QuestManager->CompleteQuestByDialogue(CurrentQuestID);
                            UE_LOG(LogTemp, Warning, TEXT("[완료체크] CompleteQuestByDialogue 결과: %d"), bSuccess);
                        }
                        else
                        {
                            UE_LOG(LogTemp, Error, TEXT("[완료체크] 조건 불일치로 완료 처리 안 됨!"));
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("[완료체크] QuestManager를 찾을 수 없음!"));
                    }
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[완료체크] bWorkshop 또는 bResult가 true라서 건너뜀"));
            }
            CheckDialogueCompletionQuest();
            CreateChoiceButtons();
        }
    }
}

void UCPNPCDialogueWidget::CheckDialogueCompletionQuest()
{
    if (bCurrentIsWorkshopQuest || bIsPotionResultDialogue) return;

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>())
        {
            if (QuestManager->GetQuestState(CurrentQuestID) == EQuestState::Accepted
                && QuestManager->GetQuestCompletionType(CurrentQuestID) == EQuestCompletionType::Dialogue)
            {
                QuestManager->CompleteQuestByDialogue(CurrentQuestID);
            }
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

    UCPNPCDialogueButtonWidget* NewButton = CreateWidget<UCPNPCDialogueButtonWidget>(this, DialogueButtonClass);
    if (NewButton) {
        NewButton->SetButtonText(FText::FromString(TEXT("네")));
        NewButton->OnButtonClickedEvent.AddDynamic(this, &UCPNPCDialogueWidget::OnChoiceSelected);
        HBox_ChoiceList->AddChild(NewButton);
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
    CheckDialogueCompletionQuest();
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
        CheckDialogueCompletionQuest();
        CreateChoiceButtons();
    }
}
