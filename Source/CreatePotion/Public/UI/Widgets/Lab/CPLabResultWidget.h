#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPLabResultWidget.generated.h"

class UCPLabResultLabelValueRowWidget;
class UTextBlock;
class UVerticalBox;

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

	// 외부에서 계산이 끝난 기본 보상과 최종 보상 문구를 표시한다.
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	void SetRewardText(const FText& InBaseRewardText, const FText& InFinalRewardText);

	// 보상 상세 목록에 가산 또는 차감 사유와 금액으로 구성된 행 하나를 추가한다.
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	void AddTipRow(const FText& InReasonText, const FText& InAmountText);
	
	//UI 내용 채우는 함수
	bool InitializeResult();

protected:
	// WBP가 제거될 때 버튼 이벤트 연결을 해제해 중복 호출과 남은 참조를 방지한다.
	virtual void UnbindEvents() override;

	// 팝업 상단에 "포션 제조 결과"
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ResultTitle;

	// 팝업 상단에 성공, 부분 성공, 실패 같은 최종 결과 등급을 표시한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Grade;

	// 보상 가산 또는 차감 사유와 금액 행들을 담는 세로 목록이다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VBox_Tips;

	// 효능 행과 보상 사유 행을 생성할 때 사용하는 공용 2열 Row WBP 클래스다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|Result|Rows")
	TSubclassOf<UCPLabResultLabelValueRowWidget> LabelValueRowWidgetClass;

	// 계산된 기본 보상 값을 표시한다. 이 위젯에서는 보상을 계산하지 않는다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_BaseReward;

	// 가산과 차감이 모두 반영된 최종 보상 값을 표시한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_FinalReward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lab|Result", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float AutoCloseDelay = 1.f;
	
private:
	// 지정한 VerticalBox에 공용 Label/Value Row WBP를 생성해 데이터와 함께 추가한다.
	void AddLabelValueRow(UVerticalBox* Container, const FText& LabelText, const FText& ValueText) const;
};
