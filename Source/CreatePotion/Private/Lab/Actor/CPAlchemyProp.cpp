// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Actor/CPAlchemyProp.h"

#include "Components/StaticMeshComponent.h"
#include "Data/CPForageableItemData.h"

// Sets default values
ACPAlchemyProp::ACPAlchemyProp()
{
	PrimaryActorTick.bCanEverTick = false;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(StaticMeshComponent);
}

void ACPAlchemyProp::InitializeFromItemData(UCPForageableItemData* ItemData)
{
	WorkingIngredient = FCPLabIngredientInstance{};

	if (!ItemData)
	{
		return;
	}

	WorkingIngredient.SourceItemData = ItemData;

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

UCPForageableItemData* ACPAlchemyProp::GetSourceItemData() const
{
	return WorkingIngredient.SourceItemData.Get();
}

int32 ACPAlchemyProp::GetEffectValue(const FGameplayTag& EffectTag) const
{
	return WorkingIngredient.GetEffectValue(EffectTag);
}


