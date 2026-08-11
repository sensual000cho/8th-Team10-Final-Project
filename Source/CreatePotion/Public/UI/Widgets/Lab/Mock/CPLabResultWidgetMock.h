#pragma once

#include "CoreMinimal.h"

class UCPLabResultWidget;

/** 목업 화면에 표시할 예시 상태다. 실제 평가 결과 계약으로 사용하지 않는다. */
enum class ECPLabResultMockOutcome : uint8
{
	Success,
	PartialSuccess,
	Failure,
};

/**
 * Result Widget에 목업 데이터를 채우는 전용 도우미다.
 * 실제 판정, 보상 계산, 런타임 결과 전달에는 사용하지 않는다.
 */
class CREATEPOTION_API FCPLabResultWidgetMock
{
public:
	static void ApplyMockResult(UCPLabResultWidget& ResultWidget, ECPLabResultMockOutcome MockOutcome);
};
