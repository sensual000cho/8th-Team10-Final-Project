#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPLabResultConditionRowWidget.generated.h"

class UTextBlock;

/** 조건 이름, 실제 값, 목표 값, 판정 상태를 표시하는 Result 행이다. */
UCLASS()
class CREATEPOTION_API UCPLabResultConditionRowWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	void SetData(
		const FText& InConditionName,
		const FText& InActualValueText,
		const FText& InTargetValueText,
		const FText& InStatusText);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ConditionName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ActualValue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_TargetValue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Status;
};
