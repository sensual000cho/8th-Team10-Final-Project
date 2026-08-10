#include "UI/Widgets/Common/Dialogue/CPNPCDialogueButtonWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UCPNPCDialogueButtonWidget::NativeConstruct() {
    Super::NativeConstruct();

    if (Button_Choice) {
        Button_Choice->OnClicked.AddDynamic(this, &UCPNPCDialogueButtonWidget::HandleButtonClicked);
    }
}

void UCPNPCDialogueButtonWidget::SetButtonText(const FText& NewText) {
    if (Text_Label) {
        Text_Label->SetText(NewText);
    }
}

void UCPNPCDialogueButtonWidget::HandleButtonClicked() {
    if (Text_Label) {
        FString TextStr = Text_Label->GetText().ToString();
        OnButtonClickedEvent.Broadcast(TextStr);
    }
}