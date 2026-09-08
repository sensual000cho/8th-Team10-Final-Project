// CPShopRowWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "Types/CPContainerTypes.h"
#include "CPShopRowWidget.generated.h"

class UImage;
class UTextBlock;
class UCPItemContainerComponent;

UCLASS()
class CREATEPOTION_API UCPShopRowWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateRow(const FContainerItem& ItemData);

public:
	// 구매 팝업 연동 시 사용할 정보 (일단 저장만)
	UPROPERTY(VisibleAnywhere, Category = "Shop")
	UCPItemContainerComponent* OwnerContainer = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Shop")
	int32 SlotGridIndex = -1;

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BuyPriceText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StockText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SellPriceText;
};
