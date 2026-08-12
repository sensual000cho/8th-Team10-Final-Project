// CPLabContainerComponent.h

#include "Components/CPLabContainerComponent.h"

UCPLabContainerComponent::UCPLabContainerComponent()
{
	ContainerType = EContainerType::Slot1D;
	MaxSlots = 3;
	MaxStack = 1;

	// 아이템 최대 사이즈에 맞게 
	Columns = 3;
	Rows = 3;
}

TArray<UCPForageableItemData*> UCPLabContainerComponent::GetIngredientsData() const
{
	TArray<UCPForageableItemData*> SelectedIngredients;

	for (const FContainerItem& Item : ContainerItems)
	{
		if (Item.ItemDataAsset)
		{
			SelectedIngredients.Add(Item.ItemDataAsset);
		}
	}

	return SelectedIngredients;
}
