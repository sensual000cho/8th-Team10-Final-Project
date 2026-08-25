#include "UI/Widgets/Lab/Helper/CPLabResultUICalc.h"

#include "GameMode/CPLabGameMode.h"
#include "Quest/QuestManager.h"
#include "UI/Widgets/Lab/CPLabResultWidget.h"

bool FCPLabResultUICalc::ApplyDeliveryResult(const FCPPotionDeliveryResult& DeliveryResult, UCPLabResultWidget* ResultWidget)
{
	if (!ResultWidget || DeliveryResult.QuestId.IsNone()) return false;

	ResultWidget->ResetResultView();
	ResultWidget->SetHeaderText(
		FText::FromString(TEXT("포션 제조 결과")), GetGradeText(DeliveryResult.DeliveryGrade));

	// 현재 계약에서 TipAmount는 세부 사유 없이 합산된 추가 보상이다.
	if (DeliveryResult.TipAmount != 0)
	{
		ResultWidget->AddTipRow(
			FText::FromString(TEXT("추가 보상 (임시)")),
			FormatGold(DeliveryResult.TipAmount, true));
	}

	// RewardAmount와 TipAmount는 판정 측에서 확정된 값이며 여기서는 표시 합계만 만든다.
	int32 FinalRewardAmount = DeliveryResult.DeliveryGrade == EDeliveryGrade::Perfect ? 
		DeliveryResult.RewardAmount + DeliveryResult.TipAmount : DeliveryResult.TipAmount;
	ResultWidget->SetRewardText(
		FormatGold(DeliveryResult.RewardAmount, false),
		FormatGold(FinalRewardAmount, false));

	return true;
}

FText FCPLabResultUICalc::FormatGold(int32 Amount, bool bShowSign)
{
	const FString AmountText = bShowSign && Amount > 0
		? FString::Printf(TEXT("+%d"), Amount)
		: FString::FromInt(Amount);
	return FText::FromString(FString::Printf(TEXT("%s G"), *AmountText));
}

FText FCPLabResultUICalc::GetGradeText(EDeliveryGrade DeliveryGrade)
{
	switch (DeliveryGrade)
	{
	case EDeliveryGrade::Perfect:
		return FText::FromString(TEXT("성공"));
	case EDeliveryGrade::Fail:
	default:
		return FText::FromString(TEXT("실패"));
	}
}
