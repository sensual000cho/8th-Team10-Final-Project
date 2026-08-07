#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h" 
#include "CPNPCDialogueButtonWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueButtonClicked, const FString&, ButtonText);

UCLASS()
class CREATEPOTION_API UCPNPCDialogueButtonWidget : public UUserWidget { 
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetButtonText(const FText& NewText);

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueButtonClicked OnButtonClickedEvent;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Choice;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_Label;

private:
    UFUNCTION()
    void HandleButtonClicked();
};