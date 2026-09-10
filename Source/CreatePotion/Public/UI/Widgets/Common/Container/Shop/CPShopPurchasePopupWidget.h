// CPShopPurchasePopupWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPShopPurchasePopupWidget.generated.h"

class UImage;
class UTextBlock;
class UEditableTextBox;
class USlider;
class UButton;
class UBorder;
class UCPItemContainerComponent;
class UCPInventoryComponent;

UCLASS()
class CREATEPOTION_API UCPShopPurchasePopupWidget : public UCPBasePopupWidget
{
	GENERATED_BODY()

public:
	// Row 클릭시 호출됨
	void OpenForPurchase(UCPItemContainerComponent* InShopContainer, int32 InGridIndex, UCPInventoryComponent* InPlayerInventory);

protected:
	virtual void NativeConstruct() override;

	// 드래그 기능
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	UFUNCTION()
	void OnQuantityChanged(float NewValue);

	UFUNCTION()
	void OnConfirmClicked();

	UFUNCTION()
	void OnCancelClicked();

	void RefreshDisplay();
	void ClosePopup();

	UFUNCTION()
	void OnPurchaseQuantityChanged(float NewValue);

	UFUNCTION()
	void OnQuantityTextCommitted(const FText& NewText, ETextCommit::Type CommitType);

private:
	UPROPERTY()
	UCPItemContainerComponent* ShopContainer = nullptr;

	UPROPERTY()
	UCPInventoryComponent* PlayerInventory = nullptr;

	int32 ShopGridIndex = -1;
	int32 CurrentQuantity = 1;

	bool bIsDraggingWindow = false;

	UPROPERTY(meta = (BindWidget))
	UBorder* DragBorder;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalPriceText;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* QuantityInputBox;

	UPROPERTY(meta = (BindWidget))
	USlider* QuantitySlider;

	UPROPERTY(meta = (BindWidget))
	UButton* ConfirmButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CancelButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ErrorMessageText;
};
