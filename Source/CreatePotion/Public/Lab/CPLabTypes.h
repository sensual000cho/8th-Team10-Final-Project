#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CPLabTypes.generated.h"

class UCPForageableItemData;

// 손님이 요청한 포션 한 건의 기본 정보
USTRUCT(BlueprintType)
struct FCPLabPotionRequest
{
	GENERATED_BODY()

	// 세션 안에서 리퀘스트를 구분하기 위한 ID
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Lab|Request")
	FName RequestId = NAME_None;

	// 플레이어에게 보여 줄 리퀘스트 내용
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Lab|Request")
	FText DisplayText;

	// 리퀘스트에 필요한 ID와 표시 문장이 모두 들어 있는지 확인
	bool IsValid() const
	{
		return !RequestId.IsNone() && !DisplayText.IsEmpty();
	}
};

// 원본 재료 정보와 현재 효과 태그를 함께 보관하는 작업용 데이터
USTRUCT(BlueprintType)
struct FCPLabIngredientInstance
{
	GENERATED_BODY()

	// 이 작업 재료가 어떤 원본 재료에서 만들어졌는지
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Ingredient")
	TObjectPtr<UCPForageableItemData> SourceItemData = nullptr;

	// 원본에서 복사한 뒤 가공에 따라 변경되는 현재 효과 태그
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Ingredient")
	TArray<FGameplayTag> CurrentEffects;

	// 사용할 수 있는 원본 재료가 연결되어 있는지 확인
	bool IsValid() const
	{
		return SourceItemData != nullptr;
	}

};
