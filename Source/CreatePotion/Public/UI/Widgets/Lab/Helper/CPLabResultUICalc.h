#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class UCPLabResultWidget;
enum class EDeliveryGrade : uint8;
struct FCPPotionDeliveryResult;

/**
 * 납품 판정 결과를 Result Widget의 표시 형식으로 변환한다.
 * 요구 조건은 UQuestManager에서 조회하고, 현재 결과는 태그 존재 여부로 표시한다.
 */
class CREATEPOTION_API FCPLabResultUICalc
{
public:
	// 계산이 끝난 납품 결과를 지정한 Result Widget에 표시한다.
	static bool ApplyDeliveryResult(const FCPPotionDeliveryResult& DeliveryResult, UCPLabResultWidget* ResultWidget);

private:
	// 보상 수치를 100 G 또는 +20 G 형태로 변환
	static FText FormatGold(int32 Amount, bool bShowSign);

	// 납품 등급을 Result Popup에서 성공/부분성공/실패 등급 문구로 변환
	static FText GetGradeText(EDeliveryGrade DeliveryGrade);
};
