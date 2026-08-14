#include "UI/Widgets/Lab/Helper/CPLabResultUICalc.h"

#include "GameMode/CPLabGameMode.h"
#include "Quest/QuestManager.h"
#include "Quest/QuestTypes.h"
#include "UI/Widgets/Lab/CPLabResultWidget.h"

bool FCPLabResultUICalc::ApplyDeliveryResult(
	const FCPPotionDeliveryResult& DeliveryResult,
	const UQuestManager* QuestManager,
	UCPLabResultWidget* ResultWidget)
{
	if (!QuestManager || !ResultWidget || DeliveryResult.QuestId.IsNone()) return false;

	ResultWidget->ResetResultView();
	ResultWidget->SetHeaderText(
		FText::FromString(TEXT("포션 제조 결과")),
		GetGradeText(DeliveryResult.DeliveryGrade));
	ResultWidget->SetDiagnosisText(GetDiagnosisText(DeliveryResult.DeliveryGrade));

	TArray<FGameplayTag> SortedCurrentEffects = DeliveryResult.CurrentEffects;
	SortedCurrentEffects.Sort([](const FGameplayTag& Left, const FGameplayTag& Right)
	{
		const int32 LeftOrder = GetEffectDisplayOrder(Left);
		const int32 RightOrder = GetEffectDisplayOrder(Right);
		return LeftOrder == RightOrder
			? Left.ToString() < Right.ToString()
			: LeftOrder < RightOrder;
	});

	for (const FGameplayTag& EffectTag : SortedCurrentEffects)
	{
		if (!EffectTag.IsValid()) continue;
		ResultWidget->AddEffectRow(
			GetEffectDisplayName(EffectTag),
			FText::FromString(TEXT("있음")));
	}

	const TArray<FQuestEffectRequirement> Requirements =
		QuestManager->GetQuestEffectRequirements(DeliveryResult.QuestId);

	for (const FQuestEffectRequirement& Requirement : Requirements)
	{
		const bool bHasCurrentEffect = DeliveryResult.CurrentEffects.Contains(Requirement.Axis);

		ResultWidget->AddConditionRow(
			GetEffectDisplayName(Requirement.Axis),
			bHasCurrentEffect ? FText::FromString(TEXT("있음")) : FText::FromString(TEXT("없음")),
			FText::FromString(TEXT("요청됨")),
			GetConditionStatusText(
				bHasCurrentEffect ? EConditionMatchResult::Correct : EConditionMatchResult::WrongTag,
				true,
				bHasCurrentEffect));
	}

	for (const FGameplayTag& EffectTag : SortedCurrentEffects)
	{
		if (!EffectTag.IsValid()) continue;

		const bool bWasRequested = Requirements.ContainsByPredicate(
			[&EffectTag](const FQuestEffectRequirement& Requirement)
			{
				return Requirement.Axis == EffectTag;
			});

		if (bWasRequested) continue;

		ResultWidget->AddConditionRow(
			GetEffectDisplayName(EffectTag),
			FText::FromString(TEXT("있음")),
			FText::FromString(TEXT("요청 없음")),
			GetConditionStatusText(
				EConditionMatchResult::WrongTag,
				false,
				true));
	}

	// 현재 계약에서 TipAmount는 세부 사유 없이 합산된 추가 보상이다.
	if (DeliveryResult.TipAmount != 0)
	{
		ResultWidget->AddTipRow(
			FText::FromString(TEXT("추가 보상 (임시)")),
			FormatGold(DeliveryResult.TipAmount, true));
	}

	// RewardAmount와 TipAmount는 판정 측에서 확정된 값이며 여기서는 표시 합계만 만든다.
	const int32 FinalRewardAmount = DeliveryResult.RewardAmount + DeliveryResult.TipAmount;
	ResultWidget->SetRewardText(
		FormatGold(DeliveryResult.RewardAmount, false),
		FormatGold(FinalRewardAmount, false));

	// 임시 표시 정책: Perfect만 성공, Okay와 Good은 부분 성공으로 취급한다.
	if (DeliveryResult.DeliveryGrade == EDeliveryGrade::Perfect)
	{
		ResultWidget->ShowConfirmAction();
	}
	else
	{
		ResultWidget->ShowRetryAndContinueActions();
	}

	return true;
}

FText FCPLabResultUICalc::GetEffectDisplayName(const FGameplayTag& EffectTag)
{
	const FString TagString = EffectTag.ToString();
	if (TagString == TEXT("Alchemy.Drowsiness"))
	{
		return FText::FromString(TEXT("수면 유도"));
	}
	if (TagString == TEXT("Alchemy.BodyHeat"))
	{
		return FText::FromString(TEXT("체온 상승"));
	}
	if (TagString == TEXT("Alchemy.Emotion"))
	{
		return FText::FromString(TEXT("정서 안정"));
	}

	FString LeftPart;
	FString RightPart;
	return FText::FromString(TagString.Split(TEXT("."), &LeftPart, &RightPart, ESearchCase::CaseSensitive, ESearchDir::FromEnd)
		? RightPart
		: TagString);
}

FText FCPLabResultUICalc::FormatSignedValue(int32 Value)
{
	return FText::FromString(Value > 0
		? FString::Printf(TEXT("+%d"), Value)
		: FString::FromInt(Value));
}

FText FCPLabResultUICalc::FormatGold(int32 Amount, bool bShowSign)
{
	const FString AmountText = bShowSign && Amount > 0
		? FString::Printf(TEXT("+%d"), Amount)
		: FString::FromInt(Amount);
	return FText::FromString(FString::Printf(TEXT("%s G"), *AmountText));
}

FText FCPLabResultUICalc::FormatTargetRange(int32 MinValue, int32 MaxValue)
{
	constexpr int32 EffectMinValue = -3;
	constexpr int32 EffectMaxValue = 3;

	if (MinValue > MaxValue)
	{
		return FText::FromString(TEXT("조건 데이터 오류"));
	}
	if (MinValue <= EffectMinValue && MaxValue >= EffectMaxValue)
	{
		return FText::FromString(TEXT("제한 없음"));
	}
	if (MinValue == MaxValue)
	{
		return FormatSignedValue(MinValue);
	}
	if (MinValue <= EffectMinValue)
	{
		return FText::FromString(FString::Printf(TEXT("%s 이하"), *FormatSignedValue(MaxValue).ToString()));
	}
	if (MaxValue >= EffectMaxValue)
	{
		return FText::FromString(FString::Printf(TEXT("%s 이상"), *FormatSignedValue(MinValue).ToString()));
	}

	return FText::FromString(FString::Printf(
		TEXT("%s ~ %s"),
		*FormatSignedValue(MinValue).ToString(),
		*FormatSignedValue(MaxValue).ToString()));
}

FText FCPLabResultUICalc::GetGradeText(EDeliveryGrade DeliveryGrade)
{
	switch (DeliveryGrade)
	{
	case EDeliveryGrade::Perfect:
		return FText::FromString(TEXT("성공"));
	case EDeliveryGrade::Good:
		return FText::FromString(TEXT("부분 성공 - Good"));
	case EDeliveryGrade::Okay:
		return FText::FromString(TEXT("부분 성공 - Okay"));
	case EDeliveryGrade::Fail:
	default:
		return FText::FromString(TEXT("실패"));
	}
}

FText FCPLabResultUICalc::GetDiagnosisText(EDeliveryGrade DeliveryGrade)
{
	switch (DeliveryGrade)
	{
	case EDeliveryGrade::Perfect:
		return FText::FromString(TEXT("요청 조건을 모두 충족했습니다."));
	case EDeliveryGrade::Good:
	case EDeliveryGrade::Okay:
		return FText::FromString(TEXT("일부 요청 조건을 충족했습니다. 세부 판정을 확인해 주세요."));
	case EDeliveryGrade::Fail:
	default:
		return FText::FromString(TEXT("요청 조건을 충족하지 못했습니다. 세부 판정을 확인해 주세요."));
	}
}

FText FCPLabResultUICalc::GetConditionStatusText(
	EConditionMatchResult MatchResult,
	bool bWasRequested,
	bool bHasCurrentEffect)
{
	if (!bWasRequested)
	{
		return FText::FromString(TEXT("요청 외 효능"));
	}
	if (!bHasCurrentEffect)
	{
		return FText::FromString(TEXT("효능 없음"));
	}

	switch (MatchResult)
	{
	case EConditionMatchResult::Correct:
		return FText::FromString(TEXT("충족"));
	case EConditionMatchResult::TooHigh:
		return FText::FromString(TEXT("초과"));
	case EConditionMatchResult::TooLow:
		return FText::FromString(TEXT("부족"));
	case EConditionMatchResult::WrongTag:
	default:
		return FText::FromString(TEXT("판정 오류"));
	}
}

int32 FCPLabResultUICalc::GetEffectDisplayOrder(const FGameplayTag& EffectTag)
{
	const FString TagString = EffectTag.ToString();
	if (TagString == TEXT("Alchemy.Drowsiness")) return 0;
	if (TagString == TEXT("Alchemy.BodyHeat")) return 1;
	if (TagString == TEXT("Alchemy.Emotion")) return 2;
	return 100;
}
