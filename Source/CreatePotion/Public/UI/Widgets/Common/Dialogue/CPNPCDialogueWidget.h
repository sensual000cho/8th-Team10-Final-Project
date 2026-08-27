#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPNPCDialogueWidget.generated.h"

class UTextBlock;
class UHorizontalBox;
class UCPNPCDialogueButtonWidget;
class UButton;

UCLASS()
class CREATEPOTION_API UCPNPCDialogueWidget : public UCPBasePopupWidget {
    GENERATED_BODY()

public:
    //UFUNCTION(BlueprintCallable, Category = "Dialogue")
   // void InitDialogue(bool bIsWorkshopQuest, FName InQuestID, const FText& InNPCName, const FText& InDialogueText, class ACPLabNPC* InSourceLabNPC = nullptr);

    // [신규] 여러 줄 대사(TArray<FText>)를 순서대로 타이핑해서 보여주기 위한 함수
    // UFUNCTION 오버로드가 블루프린트에서 지원되지 않아 이름을 다르게 지정
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void InitDialogueLines(bool bIsWorkshopQuest, FName InQuestID, const FText& InNPCName, const TArray<FText>& InDialogueLines, class ACPLabNPC* InSourceLabNPC = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void InitResultDialogue(bool bIsWorkshopQuest, FName InQuestID, const FText& InNPCName, const FText& InDialogueText, class ACPLabNPC* InSourceLabNPC, AActor* InInteractor);
protected:
    virtual void BindEvents() override;
    virtual void UnbindEvents() override;

    // 마우스 좌클릭 감지
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    // 스페이스바 등 키보드 입력 감지
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
    UFUNCTION()
    void OnChoiceSelected(const FString& ButtonText);

    void PlayTypewriterEffect(const FText& InDialogueText);
    void OnTypewriterTick();
    void CreateChoiceButtons();

    // [신규] DialogueLines 배열에서 현재 CurrentLineIndex에 해당하는 줄 하나를 꺼내
   // PlayTypewriterEffect()로 타이핑 재생
    void PlayCurrentLine();

    // [신규] 아직 보여줄 다음 줄이 남아있을 때, 기존 선택지 버튼 대신
    // "다음" 버튼 하나만 표시하기 위한 함수. ( 줄 넘기기 전용, 기존 CreateChoiceButtons와는 별개)
    void CreateContinueButton();

    // 전체 대화 스킵 버튼 클릭 시 호출
    UFUNCTION()
    void OnSkipAllClicked();

    // 진행 중인 타이핑을 즉시 끝내고 전체 텍스트를 보여주는 함수
    void SkipTypewriterEffect();

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_NPCName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_Dialogue;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> HBox_ChoiceList;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_SkipAll;

    UPROPERTY(EditDefaultsOnly, Category = "Dialogue")
    TSubclassOf<UCPNPCDialogueButtonWidget> DialogueButtonClass;


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

    bool bIsPotionResultDialogue = false; // 납품 결과 대사인지 여부

    // [신규] 여러 줄 대사 원본 배열. InitDialogue(배열 버전) 호출 시 저장됨.
    // 단일 텍스트 InitDialogue()로 호출된 경우엔 빈 배열로 초기화되어 이 기능과 무관하게 동작.
    TArray<FText> DialogueLines;

    // [신규] DialogueLines 중 현재 몇 번째 줄을 보여주고 있는지 추적하는 인덱스.
    int32 CurrentLineIndex = 0;
    
    UPROPERTY()
    TObjectPtr<AActor> ResultInteractor;
};