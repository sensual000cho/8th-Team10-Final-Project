#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h" 
#include "CPNPCDialogueButtonWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueButtonClicked, const FString&, ButtonText);

UCLASS()
class CREATEPOTION_API UCPNPCDialogueButtonWidget : public UCPBaseUserWidget {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetButtonText(const FText& NewText);

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueButtonClicked OnButtonClickedEvent;

protected:
    virtual void BindEvents() override;
    virtual void UnbindEvents() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Choice;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_Label;

private:
    UFUNCTION()
    void HandleButtonClicked();
};