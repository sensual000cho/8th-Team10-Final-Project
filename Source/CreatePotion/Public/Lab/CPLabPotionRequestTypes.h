#pragma once

#include "CoreMinimal.h"
#include "Lab/CPLabTypes.h"
#include "CPLabPotionRequestTypes.generated.h"

// 리퀘스트 하나의 제조 진행 상태
UENUM(BlueprintType)
enum class ECPLabPotionRequestPhase : uint8
{
	// 리퀘스트를 선택한 상태
	Selected,
	// 재료를 가공하는 상태
	Processing,
	// 포션 제조가 끝나 납품할 수 있는 상태
	PotionReady,
};

// 리퀘스트 하나의 진행 상태를 보관
USTRUCT(BlueprintType)
struct FCPLabPotionRequestState
{
	GENERATED_BODY()

	// 이 상태가 담당하는 포션 리퀘스트
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Request")
	FCPLabPotionRequest PotionRequest;

	// 현재 리퀘스트의 제조 진행 상태
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Request")
	ECPLabPotionRequestPhase Phase = ECPLabPotionRequestPhase::Processing;
	
	// 리퀘스트 정보가 정상인지 확인
	bool IsValid() const {
		return PotionRequest.IsValid();
	}
};

