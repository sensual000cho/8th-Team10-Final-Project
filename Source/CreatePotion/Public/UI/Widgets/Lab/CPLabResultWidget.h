#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPLabResultWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

UENUM(BlueprintType)
enum class ECPLabResultPresentationOutcome : uint8
{
	Success,
	PartialSuccess,
	Failure,
};

USTRUCT(BlueprintType)
struct FCPLabResultEffectView
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	FText EffectName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	FText ValueText;
};

USTRUCT(BlueprintType)
struct FCPLabResultConditionView
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	FText ConditionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	FText ActualValueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	FText TargetValueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	FText StatusText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	bool bShowTargetValue = true;
};

USTRUCT(BlueprintType)
struct FCPLabResultTipView
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	FText ReasonText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	FText AmountText;
};

USTRUCT(BlueprintType)
struct FCPLabResultViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	ECPLabResultPresentationOutcome Outcome = ECPLabResultPresentationOutcome::Success;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	FText ResultTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	FText GradeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	TArray<FCPLabResultEffectView> Effects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	TArray<FCPLabResultConditionView> Conditions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	FText DiagnosisText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	TArray<FCPLabResultTipView> TipItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	FText BaseRewardText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result")
	FText FinalRewardText;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCPOnLabResultActionRequested);

/**
 * Result Popup의 표시와 선택 입력만 담당한다.
 * 판정, 보상 계산, Quest 완료, 재시도 상태 전이는 외부 담당자가 처리한다.
 */
UCLASS()
class CREATEPOTION_API UCPLabResultWidget : public UCPBasePopupWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	void SetResultViewData(const FCPLabResultViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "Lab|Result|Prototype")
	void ShowMockOutcome(ECPLabResultPresentationOutcome InOutcome);

	UPROPERTY(BlueprintAssignable, Category = "Lab|Result|Actions")
	FCPOnLabResultActionRequested OnConfirmRequested;

	UPROPERTY(BlueprintAssignable, Category = "Lab|Result|Actions")
	FCPOnLabResultActionRequested OnRetryRequested;

	UPROPERTY(BlueprintAssignable, Category = "Lab|Result|Actions")
	FCPOnLabResultActionRequested OnContinueRequested;

protected:
	virtual void NativePreConstruct() override;
	virtual void BindEvents() override;
	virtual void UnbindEvents() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result|Prototype")
	bool bUseMockData = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab|Result|Prototype")
	ECPLabResultPresentationOutcome PreviewOutcome = ECPLabResultPresentationOutcome::Success;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ResultTitle;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Grade;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> VBox_Effects;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> VBox_Conditions;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Diagnosis;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> VBox_Tips;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_BaseReward;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_FinalReward;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Primary;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_PrimaryAction;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Secondary;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_SecondaryAction;

private:
	UFUNCTION()
	void HandlePrimaryAction();

	UFUNCTION()
	void HandleSecondaryAction();

	void RefreshView();
	void RebuildEffectRows();
	void RebuildConditionRows();
	void RebuildTipRows();
	void AddTwoColumnRow(UVerticalBox* Container, const FText& LeftText, const FText& RightText) const;
	void AddConditionRow(const FCPLabResultConditionView& Condition) const;

	static FCPLabResultViewData MakeMockData(ECPLabResultPresentationOutcome Outcome);

	UPROPERTY(Transient)
	FCPLabResultViewData CurrentViewData;
};
