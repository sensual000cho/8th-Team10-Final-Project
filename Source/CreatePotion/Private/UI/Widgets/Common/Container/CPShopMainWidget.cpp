// CPShopMainWidget.cpp

#include "UI/Widgets/Common/Container/CPShopMainWidget.h"
#include "Components/VerticalBox.h"

#include "UI/Widgets/Common/Container/CPShopRowWidget.h"
#include "Components/CPItemContainerComponent.h"

void UCPShopMainWidget::BindContainer(UCPItemContainerComponent* InContainer)
{
	if (!InContainer)
	{
		return;
	}

	TargetContainer = InContainer;

	TargetContainer->OnContainerUpdated.AddDynamic(this, &UCPShopMainWidget::UpdateUI);

	UpdateUI();
}

void UCPShopMainWidget::UpdateUI()
{
	if (!TargetContainer || !ShopRowContainer || !ShopRowWidgetClass)
	{
		return;
	}

	ShopRowContainer->ClearChildren();

	for (const FContainerItem& Item : TargetContainer->ContainerItems)
	{
		if (!Item.Instance.SourceItemData)
		{
			continue;
		}

		UCPShopRowWidget* Row = CreateWidget<UCPShopRowWidget>(this, ShopRowWidgetClass);
		if (Row)
		{
			Row->UpdateRow(Item);
			Row->OwnerContainer = TargetContainer;
			ShopRowContainer->AddChild(Row);
		}
	}
}