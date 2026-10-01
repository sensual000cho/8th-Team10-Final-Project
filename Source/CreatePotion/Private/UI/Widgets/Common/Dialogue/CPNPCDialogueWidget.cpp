#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueButtonWidget.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "Quest/QuestManager.h"
#include "TimerManager.h"
#include "Settings/CPUISettings.h"
#include "UI/Widgets/Lab/CPLabResultWidget.h"
#include "Kismet/GameplayStatics.h"

// [신규] DialogueLines[CurrentLineIndex]를 꺼내 타자기 효과로 재생.
// "다음" 버튼 클릭 시에도 이 함수가 다시 호출되어 다음 줄을 재생함.
void UCPNPCDialogueWidget::PlayCurrentLine()
{
    if (DialogueLines.IsValidIndex(CurrentLineIndex))
    {
        PlayTypewriterEffect(DialogueLines[CurrentLineIndex].Text);

        const FCPDialogueLine& CurrentLine = DialogueLines[CurrentLineIndex];
        if (!CurrentLine.Voice.IsNull())
        {
            if (USoundBase* SoundAsset = CurrentLine.Voice.LoadSynchronous())
            {
                UGameplayStatics::PlaySound2D(this, SoundAsset);
            }
        }
    }
}

void UCPNPCDialogueWidget::InitDialogueLines(FName InQuestID, const FText& InNPCName,
    const TArray<FCPDialogueLine>& InDialogueLines)
{
    CurrentQuestID = InQuestID;
    DialogueLines = InDialogueLines;
    CurrentLineIndex = 0;
    
    if (Text_NPCName) {
        Text_NPCName->SetText(InNPCName);
    }
    
    PlayCurrentLine();
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
    if (!GI){
        OnClosed.Broadcast();
        RequestClose();
        return;
    }
    
    if (ButtonText == TEXT("결과 보기"))
    {
        UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>();
        const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
        UClass* ResultWidgetClass = UISettings ? UISettings->ResultWidgetClass.LoadSynchronous() : nullptr;
        
        if (UIManager && ResultWidgetClass){
            UCPLabResultWidget* ResultWidget = Cast<UCPLabResultWidget>(UIManager->PushWidget(ResultWidgetClass));
            if (ResultWidget && !ResultWidget->InitializeResult(CurrentQuestID)){
                ResultWidget->RequestClose();
            }
        }
        
        OnClosed.Broadcast();
        RequestClose(); // 대화창 닫기
        return;
    }
    
    UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>();
    if (ButtonText == TEXT("네")) {
        if (QuestManager && !CurrentQuestID.IsNone() 
            && QuestManager->GetQuestState(CurrentQuestID) == EQuestState::NotAccepted) {
            QuestManager->AcceptQuest(CurrentQuestID);
        }
        OnClosed.Broadcast();
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
            GetWorld()->GetTimerManager().ClearTimer(TypewriterTimerHandle);
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
}

void UCPNPCDialogueWidget::CreateChoiceButtons()
{
    if (!HBox_ChoiceList || !DialogueButtonClass) { return; }

    HBox_ChoiceList->ClearChildren();
    FText ButtonText = FText::FromString(TEXT("네"));
    
    UGameInstance* GI = GetGameInstance();
    UQuestManager* QuestManager = GI ? GI->GetSubsystem<UQuestManager>() : nullptr;
    if (QuestManager){
        const FCPQuestResult QuestResult = QuestManager->GetResult(CurrentQuestID);
        if (!QuestResult.QuestId.IsNone()){
            ButtonText = FText::FromString(TEXT("결과 보기"));
        }
    }
    
    UCPNPCDialogueButtonWidget* NewButton = CreateWidget<UCPNPCDialogueButtonWidget>(this, DialogueButtonClass);
    if (NewButton) {
        NewButton->SetButtonText(ButtonText);
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
        FullDialogueText = DialogueLines[CurrentLineIndex].Text.ToString();
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
