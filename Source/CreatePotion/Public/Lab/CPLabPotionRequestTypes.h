#pragma once

#include "CoreMinimal.h"
#include "Lab/CPLabTypes.h"
#include "CPLabPotionRequestTypes.generated.h"

namespace CPLabPotionRequestRules
{
	// 한 리퀘스트에서 사용하는 고정 슬롯 수
	inline constexpr int32 IngredientSlotCapacity = 3;

	// 가공을 시작할 때 필요한 최소·최대 재료 수
	inline constexpr int32 MinSelectedIngredientCount = 1;
	inline constexpr int32 MaxSelectedIngredientCount = IngredientSlotCapacity;

	// 현재 프로토타입은 1개부터 시작하고 추후 최대 5개까지 사용
	inline constexpr int32 MinRequestCount = 1;
	inline constexpr int32 MaxRequestCount = 5;
}

// 포션 세션 전체의 진행 상태
UENUM(BlueprintType)
enum class ECPLabPotionSessionPhase : uint8
{
	// 아직 벨을 울리지 않은 상태
	WaitingForBell,
	// 하나 이상의 리퀘스트를 제조 중인 상태
	InProgress,
	// 세션의 모든 리퀘스트를 납품한 상태
	Completed,
};

// 리퀘스트 하나의 제조 진행 상태
UENUM(BlueprintType)
enum class ECPLabPotionRequestPhase : uint8
{
	// 아직 수락하지 않은 대기 상태
	Queued,
	// 재료를 슬롯에 준비하는 상태
	Preparing,
	// 준비한 재료를 가공하는 상태
	Processing,
	// 포션 제조가 끝나 납품할 수 있는 상태
	PotionReady,
	// 손님에게 포션을 전달한 상태
	Delivered,
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
	ECPLabPotionRequestPhase Phase = ECPLabPotionRequestPhase::Queued;
	
	// 리퀘스트 정보가 정상인지 확인
	bool IsValid() const {
		return PotionRequest.IsValid();
	}
};

// 공방에 들어온 여러 포션 리퀘스트와 세션 진행 상태를 보관
USTRUCT(BlueprintType)
struct FCPLabPotionSessionState
{
	GENERATED_BODY()

	// 포션 세션 전체의 현재 진행 상태
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Session")
	ECPLabPotionSessionPhase Phase = ECPLabPotionSessionPhase::WaitingForBell;

	// 지금 플레이어가 진행 중인 리퀘스트 ID
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Session")
	FName ActiveRequestId = NAME_None;

	// 세션에 들어온 리퀘스트별 제조 상태
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Session")
	TArray<FCPLabPotionRequestState> RequestStates;

	// 리퀘스트 수와 현재 활성 상태가 서로 맞는지 확인
	bool IsValid() const
	{
		// 세션에 들어온 요청 개수가 허용 범위 인지 확인
		const int32 RequestCount = RequestStates.Num();
		if (RequestCount < CPLabPotionRequestRules::MinRequestCount || 
			RequestCount > CPLabPotionRequestRules::MaxRequestCount)
		{
			return false;
		}
		
		// 각 요청 상태가 정상인지 확인
		for (const FCPLabPotionRequestState& RequestState : RequestStates){
			if (!RequestState.IsValid()) return false;
		}
		
		// 세션 진행 중 -> 활성 request가 있어야 함
		if (Phase == ECPLabPotionSessionPhase::InProgress){
			if (ActiveRequestId.IsNone()) return false;
			
			// 활성 request가 존재하는지 확인
			for (const FCPLabPotionRequestState& RequestState : RequestStates){
				if (ActiveRequestId == RequestState.PotionRequest.RequestId) return true;
			}
			
			return false;
		// 세션 완료 -> 활성 request가 없어야 함
		} else if (Phase == ECPLabPotionSessionPhase::Completed){
			if (!ActiveRequestId.IsNone()) return false;
			
			// 모든 request가 완료 상태인지 확인
			for (const FCPLabPotionRequestState& RequestState : RequestStates){
				if ( RequestState.Phase != ECPLabPotionRequestPhase::Delivered) return false;
			}
			
			return true;
		}

		return false;
	}
};
