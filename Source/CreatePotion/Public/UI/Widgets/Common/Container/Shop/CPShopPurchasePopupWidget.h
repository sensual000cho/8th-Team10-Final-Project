// CPShopPurchasePopupWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "Types/CPContainerTypes.h"
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
	// 상점 컨테이너 Row 클릭시 호출
	void OpenForPurchase(UCPItemContainerComponent* InShopContainer, int32 InGridIndex, UCPInventoryComponent* InPlayerInventory);

	void OpenForSale(UCPItemContainerComponent* InPlayerContainer, int32 InGridIndex, UCPItemContainerComponent* InShopContainer, UCPInventoryComponent* InPlayerInventory);

	void OpenForSplit(UCPItemContainerComponent* InSourceContainer, int32 InGridIndex);

	void PlaceNearCursor();
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

	void ChangeConfirmText();

	UFUNCTION()
	void OnCancelClicked();

	void RefreshDisplay();
	void ClosePopup();

	UFUNCTION()
	void OnPurchaseQuantityChanged(float NewValue);

	UFUNCTION()
	void OnQuantityTextCommitted(const FText& NewText, ETextCommit::Type CommitType);

private:
	EShopPopupMode CurrentMode = EShopPopupMode::Buy;

	// 판매 모드일 때 필요한 추가 참조 (구매 모드에선 필요 x)
	UPROPERTY()
	UCPItemContainerComponent* PlayerContainerForSale = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ModeTitleText;

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

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ConfirmButtonText;

	UPROPERTY(meta = (BindWidget))
	UButton* CancelButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ErrorMessageText;
};
