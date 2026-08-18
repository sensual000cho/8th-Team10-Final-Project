#include "Lab/Actor/CPAlchemyProp.h"

#include "Components/StaticMeshComponent.h"
#include "Data/CPForageableItemData.h"

// 물리 재료 Actor의 기본 컴포넌트 구성
ACPAlchemyProp::ACPAlchemyProp(): ProcessMultiplier(1) 
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

	// DataAsset의 원본 효과 태그를 복사해 Actor만의 작업값 생성
	for (const FGameplayTag& EffectTag : ItemData->TagAxes){
		if (!EffectTag.IsValid())continue;

		WorkingIngredient.CurrentEffects.Add(EffectTag);
	}
	
	OnAlchemyPropChanged.Broadcast();
}

void ACPAlchemyProp::InitializeFromEffects(UCPForageableItemData* ItemData, const TArray<FGameplayTag>& Effects)
{
	ResetWorkingIngredient();
	
	if (!ItemData){
		OnAlchemyPropChanged.Broadcast();
		return;
	}
	
	WorkingIngredient.SourceItemData = ItemData;
	for (const FGameplayTag& EffectTag : Effects){
		if (!EffectTag.IsValid()) continue;
		
		WorkingIngredient.CurrentEffects.Add(EffectTag);
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

bool ACPAlchemyProp::HasBeenProcessedBy(FName InProcessorId) const
{
	return !InProcessorId.IsNone() && AppliedProcessorIds.Contains(InProcessorId);
}

bool ACPAlchemyProp::MarkProcessedBy(FName InProcessorId)
{
	if (InProcessorId.IsNone() || AppliedProcessorIds.Contains(InProcessorId)) return false;
	
	AppliedProcessorIds.Add(InProcessorId);
	return true;
}

bool ACPAlchemyProp::ResetToItemData()
{
	UCPForageableItemData* ItemData = GetSourceItemData();
	if (!ItemData) return false;
	
	InitializeFromItemData(ItemData);
	return true;
}

float ACPAlchemyProp::GetProcessMultiplier() const
{
	return ProcessMultiplier;
}

void ACPAlchemyProp::SetProcessMultiplier(float InMultiplier)
{
	ProcessMultiplier = InMultiplier;
}


void ACPAlchemyProp::ResetWorkingIngredient()
{
	// 다른 재료로 다시 초기화할 때 이전 재료 정보가 남지 않도록 초기화
	WorkingIngredient = FCPLabIngredientInstance{};
	AppliedProcessorIds.Reset();
	ProcessMultiplier = 1;
}


