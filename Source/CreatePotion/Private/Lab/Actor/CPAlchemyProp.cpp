#include "Lab/Actor/CPAlchemyProp.h"

#include "Components/StaticMeshComponent.h"
#include "Data/CPForageableItemData.h"

// 물리 재료 Actor의 기본 컴포넌트 구성
ACPAlchemyProp::ACPAlchemyProp()
{
	// 직접 이동·가공 요청을 받을 때만 갱신하므로 Tick은 사용하지 않음
	PrimaryActorTick.bCanEverTick = false;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(StaticMeshComponent);
}

void ACPAlchemyProp::InitializeFromItemData(UCPForageableItemData* ItemData)
{
	ResetWorkingIngredient();

	if (!ItemData)
	{
		return;
	}

	WorkingIngredient.SourceItemData = ItemData;

	// DataAsset의 원본 효과값을 복사해 Actor만의 작업값 생성
	for (const FAlchemyProperty& Property : ItemData->TagAxes)
	{
		if (!Property.Tag.IsValid())
		{
			continue;
		}

		WorkingIngredient.CurrentEffects.Add(
			Property.Tag,
			Property.Value);
	}
}

bool ACPAlchemyProp::InitializeFromRequestSlot(
	FName InRequestId,
	int32 InSourceSlotIndex,
	const FCPLabIngredientInstance& Ingredient)
{
	ResetWorkingIngredient();

	if (InRequestId.IsNone() ||
		InSourceSlotIndex < 0 ||
		InSourceSlotIndex >=
			CPLabPotionRequestRules::IngredientSlotCapacity ||
		!Ingredient.IsValid())
	{
		return false;
	}

	// 작업을 마친 뒤 원래 슬롯에 돌아갈 수 있도록 출처도 함께 저장
	SourceRequestId = InRequestId;
	SourceSlotIndex = InSourceSlotIndex;
	WorkingIngredient = Ingredient;
	return true;
}

FCPLabIngredientInstance
ACPAlchemyProp::GetWorkingIngredient() const
{
	return WorkingIngredient;
}

UCPForageableItemData* ACPAlchemyProp::GetSourceItemData() const
{
	return WorkingIngredient.SourceItemData.Get();
}

int32 ACPAlchemyProp::GetEffectValue(const FGameplayTag& EffectTag) const
{
	return WorkingIngredient.GetEffectValue(EffectTag);
}

FName ACPAlchemyProp::GetSourceRequestId() const
{
	return SourceRequestId;
}

int32 ACPAlchemyProp::GetSourceSlotIndex() const
{
	return SourceSlotIndex;
}

bool ACPAlchemyProp::IsAssignedToRequestSlot() const
{
	return !SourceRequestId.IsNone() &&
		SourceSlotIndex != INDEX_NONE;
}

void ACPAlchemyProp::ResetWorkingIngredient()
{
	// 다른 재료로 다시 초기화할 때 이전 재료 정보가 남지 않도록 전부 초기화
	WorkingIngredient = FCPLabIngredientInstance{};
	SourceRequestId = NAME_None;
	SourceSlotIndex = INDEX_NONE;
}


