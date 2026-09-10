// CPShopMainWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Common/Container/CPContainerMainWidget.h"
#include "CPShopMainWidget.generated.h"

class UVerticalBox;
class UCPShopRowWidget;
class UCPShopPurchasePopupWidget;

UCLASS()
class CREATEPOTION_API UCPShopMainWidget : public UCPContainerMainWidget
{
	GENERATED_BODY()

public:
	virtual void BindContainer(UCPItemContainerComponent* InContainer) override;
	virtual void UpdateUI() override;

	void OpenPurchasePopup(UCPItemContainerComponent* ShopContainer, int32 GridIndex);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Shop|UI")
	TSubclassOf<UCPShopRowWidget> ShopRowWidgetClass;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ShopRowContainer;

	UPROPERTY(EditDefaultsOnly, Category = "Shop|UI")
	TSubclassOf<UCPShopPurchasePopupWidget> PurchasePopupWidgetClass;

private:
	UPROPERTY()
	UCPShopPurchasePopupWidget* PurchasePopupWidgetInstance;
	
};
