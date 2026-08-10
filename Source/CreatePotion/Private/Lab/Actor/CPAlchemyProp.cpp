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

	if (!ItemData){
		OnAlchemyPropChanged.Broadcast();
		return;
	}

	WorkingIngredient.SourceItemData = ItemData;

	// DataAsset의 원본 효과값을 복사해 Actor만의 작업값 생성
	for (const FAlchemyProperty& Property : ItemData->TagAxes){
		if (!Property.Tag.IsValid())continue;

		WorkingIngredient.CurrentEffects.Add(Property.Tag, Property.Value);
	}
	
	OnAlchemyPropChanged.Broadcast();
}

FCPLabIngredientInstance
ACPAlchemyProp::GetWorkingIngredient() const
{
	return WorkingIngredient;
}

bool ACPAlchemyProp::SetWorkingIngredient(const FCPLabIngredientInstance& Ingredient)
{
	if (!Ingredient.IsValid()) return false;
	
	if (WorkingIngredient.IsValid() && WorkingIngredient.SourceItemData != Ingredient.SourceItemData) return false;
	
	WorkingIngredient = Ingredient;
	OnAlchemyPropChanged.Broadcast();
	return true;
}

UCPForageableItemData* ACPAlchemyProp::GetSourceItemData() const
{
	return WorkingIngredient.SourceItemData.Get();
}

int32 ACPAlchemyProp::GetEffectValue(const FGameplayTag& EffectTag) const
{
	return WorkingIngredient.GetEffectValue(EffectTag);
}

void ACPAlchemyProp::ResetWorkingIngredient()
{
	// 다른 재료로 다시 초기화할 때 이전 재료 정보가 남지 않도록 초기화
	WorkingIngredient = FCPLabIngredientInstance{};
}


