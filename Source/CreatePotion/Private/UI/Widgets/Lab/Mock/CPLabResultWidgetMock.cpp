#include "UI/Widgets/Lab/Mock/CPLabResultWidgetMock.h"

#include "UI/Widgets/Lab/CPLabResultWidget.h"

void FCPLabResultWidgetMock::ApplyMockResult(
	UCPLabResultWidget& ResultWidget,
	ECPLabResultMockOutcome MockOutcome)
{
	ResultWidget.ResetResultView();

	const bool bIsPartialSuccess = MockOutcome == ECPLabResultMockOutcome::PartialSuccess;
	const bool bIsFailure = MockOutcome == ECPLabResultMockOutcome::Failure;
	const FText SleepValue = FText::FromString(bIsFailure ? TEXT("0") : TEXT("+2"));
	const FText WarmthValue = FText::FromString(bIsFailure ? TEXT("-1") : bIsPartialSuccess ? TEXT("0") : TEXT("+1"));
	const FText SleepStatus = FText::FromString(bIsFailure ? TEXT("부족") : TEXT("충족"));
	const FText WarmthStatus = FText::FromString(
		bIsFailure || bIsPartialSuccess ? TEXT("부족") : TEXT("충족"));

	ResultWidget.AddEffectRow(FText::FromString(TEXT("잠이 잘 옴")), SleepValue);
	ResultWidget.AddEffectRow(FText::FromString(TEXT("몸이 따뜻해짐")), WarmthValue);
	ResultWidget.AddEffectRow(FText::FromString(TEXT("마음이 편안해짐")), FText::FromString(TEXT("+1")));

	ResultWidget.AddConditionRow(
		FText::FromString(TEXT("수면 유도")),
		SleepValue,
		FText::FromString(TEXT("Lv.2 이상")),
		SleepStatus);
	ResultWidget.AddConditionRow(
		FText::FromString(TEXT("체온 상승")),
		WarmthValue,
		FText::FromString(TEXT("Lv.1 이상")),
		WarmthStatus);
	ResultWidget.AddConditionRow(
		FText::FromString(TEXT("정서 안정")),
		FText::FromString(TEXT("+1")),
		FText::FromString(TEXT("Lv.1 이상")),
		FText::FromString(TEXT("충족")));

	if (MockOutcome == ECPLabResultMockOutcome::Success)
	{
		ResultWidget.SetHeaderText(FText::FromString(TEXT("포션 제조 결과")), FText::FromString(TEXT("성공")));
		ResultWidget.SetDiagnosisText(FText::FromString(TEXT("요청한 효능이 모두 확인되었습니다.")));
		ResultWidget.AddTipRow(FText::FromString(TEXT("신속한 제조")), FText::FromString(TEXT("+24 G")));
		ResultWidget.AddTipRow(FText::FromString(TEXT("힌트 미사용")), FText::FromString(TEXT("+12 G")));
		ResultWidget.SetRewardText(FText::FromString(TEXT("120 G")), FText::FromString(TEXT("156 G")));
		ResultWidget.ShowConfirmAction();
		return;
	}

	if (MockOutcome == ECPLabResultMockOutcome::PartialSuccess)
	{
		ResultWidget.SetHeaderText(
			FText::FromString(TEXT("포션 제조 결과")),
			FText::FromString(TEXT("부분 성공 (목업)")));
		ResultWidget.SetDiagnosisText(FText::FromString(TEXT("체온 관련 효능이 부족한 목업 결과입니다.")));
		ResultWidget.SetRewardText(FText::FromString(TEXT("80 G")), FText::FromString(TEXT("80 G")));
		ResultWidget.ShowRetryAndContinueActions();
		return;
	}

	ResultWidget.SetHeaderText(
		FText::FromString(TEXT("포션 제조 결과")),
		FText::FromString(TEXT("실패 (목업)")));
	ResultWidget.SetDiagnosisText(FText::FromString(TEXT("요청과 맞지 않는 목업 결과입니다.")));
	ResultWidget.SetRewardText(FText::FromString(TEXT("0 G")), FText::FromString(TEXT("0 G")));
	ResultWidget.ShowRetryAndContinueActions();
}
