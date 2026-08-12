#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class UCPLabResultWidget;
class UQuestManager;
enum class EConditionMatchResult : uint8;
enum class EDeliveryGrade : uint8;
struct FAlchemyProperty;
struct FCPPotionDeliveryResult;

/**
 * 납품 판정 결과를 Result Widget의 표시 형식으로 변환한다.
 * 조건 판정은 직접 계산하지 않고 UQuestManager::EvaluateConditions 결과를 사용한다.
 */
class CREATEPOTION_API FCPLabResultUICalc
{
public:
	// 계산이 끝난 납품 결과를 지정한 Result Widget에 표시한다.
	static bool ApplyDeliveryResult(
		const FCPPotionDeliveryResult& DeliveryResult,
		const UQuestManager* QuestManager,
		UCPLabResultWidget* ResultWidget);

private:
	// GameplayTag를 플레이어가 읽을 수 있는 이름으로 변환
	static FText GetEffectDisplayName(const FGameplayTag& EffectTag);

	// 효과 수치를 +2, 0, -1 형태로 변환
	static FText FormatSignedValue(int32 Value);

	// 보상 수치를 100 G 또는 +20 G 형태로 변환
	static FText FormatGold(int32 Amount, bool bShowSign);

	// 최소·최대 목표를 하나의 표시 문구로 변환한다.
	static FText FormatTargetRange(int32 MinValue, int32 MaxValue);

	// 납품 등급을 Result Popup에서 성공/부분성공/실패 등급 문구로 변환
	static FText GetGradeText(EDeliveryGrade DeliveryGrade);

	// 납품 등급에 대응하는 성공/실패 문구를 상세하게 안내해 준다
	static FText GetDiagnosisText(EDeliveryGrade DeliveryGrade);

	// 조건 판정과 효능 존재 여부를 충족, 부족, 초과, 효능 없음, 요청 외 효능 같은 문구로
	static FText GetConditionStatusText(
		EConditionMatchResult MatchResult,
		bool bWasRequested,
		bool bHasCurrentEffect);

	// Effect 배열에서 같은 Tag를 가진 항목을 찾는다.
	static const FAlchemyProperty* FindEffect(
		const TArray<FAlchemyProperty>& Effects,
		const FGameplayTag& EffectTag);

	// 최종 효능 목록의 고정 표시 순서를 반환한다.
	static int32 GetEffectDisplayOrder(const FGameplayTag& EffectTag);
};
