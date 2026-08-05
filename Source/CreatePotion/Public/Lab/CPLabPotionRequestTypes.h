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
	inline constexpr int32 MaxSelectedIngredientCount =
		IngredientSlotCapacity;

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

// 리퀘스트 하나의 진행 상태와 재료 슬롯을 보관
USTRUCT(BlueprintType)
struct FCPLabPotionRequestState
{
	GENERATED_BODY()

	// 리퀘스트를 만들 때 항상 슬롯 3칸을 준비
	FCPLabPotionRequestState()
	{
		IngredientSlots.SetNum(
			CPLabPotionRequestRules::IngredientSlotCapacity);
	}

	// 이 상태가 담당하는 포션 리퀘스트
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Request")
	FCPLabPotionRequest PotionRequest;

	// 현재 리퀘스트의 제조 진행 상태
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Request")
	ECPLabPotionRequestPhase Phase =
		ECPLabPotionRequestPhase::Queued;

	// 선택된 재료를 확정해서 보관하는 슬롯 3칸
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Request")
	TArray<FCPLabIngredientInstance> IngredientSlots;

	// 리퀘스트 정보와 슬롯 수가 정상인지 확인
	bool IsValid() const
	{
		return PotionRequest.IsValid() &&
			IngredientSlots.Num() ==
				CPLabPotionRequestRules::IngredientSlotCapacity;
	}

	// 슬롯에 실제로 들어 있는 재료 수를 계산
	int32 GetSelectedIngredientCount() const
	{
		int32 SelectedCount = 0;
		for (const FCPLabIngredientInstance& Ingredient : IngredientSlots)
		{
			if (Ingredient.IsValid())
			{
				++SelectedCount;
			}
		}

		return SelectedCount;
	}

	// 현재 재료 수로 가공을 시작할 수 있는지 확인
	bool HasValidIngredientSelection() const
	{
		const int32 SelectedCount = GetSelectedIngredientCount();
		return SelectedCount >=
				CPLabPotionRequestRules::MinSelectedIngredientCount &&
			SelectedCount <=
				CPLabPotionRequestRules::MaxSelectedIngredientCount;
	}
};

// 공방에 들어온 여러 포션 리퀘스트와 세션 진행 상태를 보관
USTRUCT(BlueprintType)
struct FCPLabPotionSessionState
{
	GENERATED_BODY()

	// 포션 세션 전체의 현재 진행 상태
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Session")
	ECPLabPotionSessionPhase Phase =
		ECPLabPotionSessionPhase::WaitingForBell;

	// 지금 플레이어가 진행 중인 리퀘스트 ID
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Session")
	FName ActiveRequestId = NAME_None;

	// 세션에 들어온 리퀘스트별 제조 상태
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Session")
	TArray<FCPLabPotionRequestState> RequestStates;

	// 리퀘스트 수와 현재 활성 상태가 서로 맞는지 확인
	bool IsValid() const
	{
		if (RequestStates.Num() <
				CPLabPotionRequestRules::MinRequestCount ||
			RequestStates.Num() >
				CPLabPotionRequestRules::MaxRequestCount ||
			RequestStates.ContainsByPredicate(
				[](const FCPLabPotionRequestState& RequestState)
				{
					return !RequestState.IsValid();
				}))
		{
			return false;
		}

		if (Phase == ECPLabPotionSessionPhase::InProgress)
		{
			return !ActiveRequestId.IsNone() &&
				RequestStates.ContainsByPredicate(
					[this](const FCPLabPotionRequestState& RequestState)
					{
						return RequestState.PotionRequest.RequestId ==
							ActiveRequestId;
					});
		}

		if (Phase == ECPLabPotionSessionPhase::Completed)
		{
			return ActiveRequestId.IsNone() &&
				!RequestStates.ContainsByPredicate(
					[](const FCPLabPotionRequestState& RequestState)
					{
						return RequestState.Phase !=
							ECPLabPotionRequestPhase::Delivered;
					});
		}

		return false;
	}
};
