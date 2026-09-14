// CPShopRowWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "Types/CPContainerTypes.h"
#include "CPShopRowWidget.generated.h"

class UImage;
class UTextBlock;

class UCPShopMainWidget;
class UCPItemContainerComponent;

UCLASS()
class CREATEPOTION_API UCPShopRowWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateRow(const FContainerItem& ItemData);

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	// 현재 이 Row가 속한 상점 위젯 - 구매 위젯 팝업에 사용
	UPROPERTY()
	UCPShopMainWidget* OwnerShopWidget = nullptr;

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
