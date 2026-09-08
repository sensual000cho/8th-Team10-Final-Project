// CPShopRowWidget.cpp

#include "UI/Widgets/Common/Container/CPShopRowWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "Data/CPForageableItemData.h"

void UCPShopRowWidget::UpdateRow(const FContainerItem& ItemData)
{
	SlotGridIndex = ItemData.GridIndex;

	UCPForageableItemData* Data = ItemData.Instance.SourceItemData;
	if (!Data)
	{
		return;
	}

	if (ItemIcon)
	{
		if (UTexture2D* LoadedTexture = Data->Icon.LoadSynchronous())
		{
			ItemIcon->SetBrushFromTexture(LoadedTexture);
		}
	}

	if (BuyPriceText)
	{
		BuyPriceText->SetText(FText::AsNumber(Data->BuyPrice));
	}

	if (StockText)
	{
		StockText->SetText(FText::AsNumber(ItemData.Stacked));
	}

	if (SellPriceText)
	{
		SellPriceText->SetText(FText::AsNumber(Data->GetSellPrice()));
	}
}
