#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPLabResultWidget.generated.h"

class UButton;
class UCPLabResultConditionRowWidget;
class UCPLabResultLabelValueRowWidget;
class UTextBlock;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCPOnLabResultActionRequested);

/**
 * 계산이 끝난 결과를 화면에 표시하고 사용자 선택을 외부로 전달한다.
 * 판정, 보상 계산, 퀘스트 처리, 재시도 초기화는 담당하지 않는다.
 */

struct FCPPotionDeliveryResult;

UCLASS()
class CREATEPOTION_API UCPLabResultWidget : public UCPBasePopupWidget
{
	GENERATED_BODY()

public:
	// 리셋 함수
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	void ResetResultView();

	// 외부에서 전달받은 결과 화면 제목과 최종 등급 문구를 상단에 표시한다.
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	void SetHeaderText(const FText& InResultTitle, const FText& InGradeText);

	// 외부 평가 결과에 포함된 진단 또는 실패 원인 문구를 표시한다.
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	void SetDiagnosisText(const FText& InDiagnosisText);

	// 외부에서 계산이 끝난 기본 보상과 최종 보상 문구를 표시한다.
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	void SetRewardText(const FText& InBaseRewardText, const FText& InFinalRewardText);

	// 발현 효능 목록에 이름과 결과 값으로 구성된 행 하나를 추가한다.
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	void AddEffectRow(const FText& InEffectName, const FText& InValueText);

	// 조건 목록에 조건 이름, 실제 값, 목표 값, 판정으로 구성된 행 하나를 추가한다.
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	void AddConditionRow(
		const FText& InConditionName,
		const FText& InActualValueText,
		const FText& InTargetValueText,
		const FText& InStatusText);

	// 보상 상세 목록에 가산 또는 차감 사유와 금액으로 구성된 행 하나를 추가한다.
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	void AddTipRow(const FText& InReasonText, const FText& InAmountText);

	// 성공 결과용으로 확인 버튼 표시하고 주 버튼 클릭을 확인 요청으로 설정한다.
	UFUNCTION(BlueprintCallable, Category = "Lab|Result|Actions")
	void ShowConfirmAction();

	// 부분 성공과 실패용으로 재시도와 넘어가기 버튼을 표시한다.
	UFUNCTION(BlueprintCallable, Category = "Lab|Result|Actions")
	void ShowRetryAndContinueActions();

	// 성공 결과에서 확인 버튼을 눌렀을 때 외부 처리 담당자에게 알린다.
	UPROPERTY(BlueprintAssignable, Category = "Lab|Result|Actions")
	FCPOnLabResultActionRequested OnConfirmRequested;

	// 부분 성공 또는 실패 결과에서 재시도 버튼을 눌렀을 때 외부 처리 담당자에게 알린다.
	UPROPERTY(BlueprintAssignable, Category = "Lab|Result|Actions")
	FCPOnLabResultActionRequested OnRetryRequested;

	// 부분 성공 또는 실패 결과에서 넘어가기 버튼을 눌렀을 때 외부 처리 담당자에게 알린다.
	UPROPERTY(BlueprintAssignable, Category = "Lab|Result|Actions")
	FCPOnLabResultActionRequested OnContinueRequested;
	
	//UI 내용 채우는 함수
	bool InitializeResult(const FCPPotionDeliveryResult& DeliveryResult);

protected:
	// WBP 생성이 끝난 뒤 두 버튼의 OnClicked 이벤트를 C++ 처리 함수에 연결한다.
	virtual void BindEvents() override;

	// WBP가 제거될 때 버튼 이벤트 연결을 해제해 중복 호출과 남은 참조를 방지한다.
	virtual void UnbindEvents() override;

	// 팝업 상단에 "포션 제조 결과"
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ResultTitle;

	// 팝업 상단에 성공, 부분 성공, 실패 같은 최종 결과 등급을 표시한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Grade;

	// 발현된 효능 행들을 순서대로 담는 세로 목록이다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VBox_Effects;

	// 의뢰 조건의 실제 값, 목표 값, 판정 결과 행들을 담는 세로 목록이다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VBox_Conditions;

	// 조건을 만족하거나 만족하지 못한 이유를 설명하는 진단 문구를 표시한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Diagnosis;

	// 보상 가산 또는 차감 사유와 금액 행들을 담는 세로 목록이다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VBox_Tips;

	// 효능 행과 보상 사유 행을 생성할 때 사용하는 공용 2열 Row WBP 클래스다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|Result|Rows")
	TSubclassOf<UCPLabResultLabelValueRowWidget> LabelValueRowWidgetClass;

	// 조건 이름, 실제 값, 목표 값, 판정을 표시하는 4열 Row WBP 클래스다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|Result|Rows")
	TSubclassOf<UCPLabResultConditionRowWidget> ConditionRowWidgetClass;

	// 계산된 기본 보상 값을 표시한다. 이 위젯에서는 보상을 계산하지 않는다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_BaseReward;

	// 가산과 차감이 모두 반영된 최종 보상 값을 표시한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_FinalReward;

	// 성공에서는 확인, 부분 성공과 실패에서는 재시도로 사용하는 주 버튼이다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Primary;

	// 주 버튼 안에 확인 또는 재시도 문구를 표시한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_PrimaryAction;

	// 부분 성공과 실패에서만 표시되는 넘어가기 버튼이다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Secondary;

	// 보조 버튼 안에 넘어가기 문구를 표시한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_SecondaryAction;


	
private:
	// 주 버튼의 현재 역할에 따라 확인 또는 재시도 Delegate를 호출한다.
	UFUNCTION()
	void HandlePrimaryAction();

	// 보조 버튼 클릭을 넘어가기 요청으로 외부 처리 담당자에게 전달한다.
	UFUNCTION()
	void HandleSecondaryAction();

	// 지정한 VerticalBox에 공용 Label/Value Row WBP를 생성해 데이터와 함께 추가한다.
	void AddLabelValueRow(UVerticalBox* Container, const FText& LabelText, const FText& ValueText) const;

	// true면 주 버튼 클릭을 확인 요청으로, false면 재시도 요청으로 전달한다.
	bool bPrimaryActionIsConfirm = true;
};
