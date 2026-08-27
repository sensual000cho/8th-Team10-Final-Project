#pragma once

#include "CoreMinimal.h"
#include "Lab/CPLabTypes.h"
#include "CPLabPotionRequestTypes.generated.h"

// 리퀘스트 하나의 진행 상태를 보관
USTRUCT(BlueprintType)
struct FCPLabPotionRequestState
{
	GENERATED_BODY()

	// 이 상태가 담당하는 포션 리퀘스트
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Request")
	FCPLabPotionRequest PotionRequest;
	
	// 리퀘스트 정보가 정상인지 확인
	bool IsValid() const {
		return PotionRequest.IsValid();
	}
};

