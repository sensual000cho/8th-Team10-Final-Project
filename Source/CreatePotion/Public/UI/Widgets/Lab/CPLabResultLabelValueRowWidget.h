#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPLabResultLabelValueRowWidget.generated.h"

class UTextBlock;

/** 효능과 가산 보상처럼 이름과 값으로 구성된 Result 행이다. */
UCLASS()
class CREATEPOTION_API UCPLabResultLabelValueRowWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	void SetData(const FText& InLabelText, const FText& InValueText);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Label;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Value;
};
