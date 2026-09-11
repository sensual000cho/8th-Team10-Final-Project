// CPShopMainWidget.cpp

#include "UI/Widgets/Common/Container/CPShopMainWidget.h"
#include "Components/VerticalBox.h"

#include "CreatePotion.h"
#include "Components/CPItemContainerComponent.h"
#include "Character/CPPlayerController.h"
#include "UI/Widgets/Common/Container/CPShopRowWidget.h"
#include "UI/Widgets/Common/Container/Shop/CPShopPurchasePopupWidget.h"


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
			Row->OwnerShopWidget = this;   // 추가
			ShopRowContainer->AddChild(Row);
		}
	}
}

void UCPShopMainWidget::OpenPurchasePopup(UCPItemContainerComponent* ShopContainer, int32 GridIndex)
{
	ACPPlayerController* PC = Cast<ACPPlayerController>(GetOwningPlayer());
	if (!PC || !PC->CachedInventoryComponent)
	{
		return;
	}

	if (UCPShopPurchasePopupWidget* Popup = PC->GetOrCreateShopPopup())
	{
		Popup->OpenForPurchase(ShopContainer, GridIndex, PC->CachedInventoryComponent);
	}
}
