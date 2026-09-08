// CPShopMainWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Common/Container/CPContainerMainWidget.h"
#include "CPShopMainWidget.generated.h"

class UVerticalBox;
class UCPShopRowWidget;

UCLASS()
class CREATEPOTION_API UCPShopMainWidget : public UCPContainerMainWidget
{
	GENERATED_BODY()

public:
	virtual void BindContainer(UCPItemContainerComponent* InContainer) override;
	virtual void UpdateUI() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Shop|UI")
	TSubclassOf<UCPShopRowWidget> ShopRowWidgetClass;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ShopRowContainer;
	
};
