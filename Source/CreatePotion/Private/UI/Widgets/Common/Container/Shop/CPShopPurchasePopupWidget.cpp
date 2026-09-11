// CPShopPurchasePopupWidget.cpp

#include "UI/Widgets/Common/Container/Shop/CPShopPurchasePopupWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Blueprint/WidgetLayoutLibrary.h"

#include "CreatePotion.h"
#include "Components/CPItemContainerComponent.h"
#include "Components/CPInventoryComponent.h"
#include "Data/CPForageableItemData.h"

void UCPShopPurchasePopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Collapsed);

	if (QuantitySlider) QuantitySlider->OnValueChanged.AddDynamic(this, &UCPShopPurchasePopupWidget::OnQuantityChanged);
	if (QuantityInputBox) QuantityInputBox->OnTextCommitted.AddDynamic(this, &UCPShopPurchasePopupWidget::OnQuantityTextCommitted);
	if (ConfirmButton) ConfirmButton->OnClicked.AddDynamic(this, &UCPShopPurchasePopupWidget::OnConfirmClicked);
	if (CancelButton) CancelButton->OnClicked.AddDynamic(this, &UCPShopPurchasePopupWidget::OnCancelClicked);
	if (CloseButton) CloseButton->OnClicked.AddDynamic(this, &UCPShopPurchasePopupWidget::OnCancelClicked);
}

FReply UCPShopPurchasePopupWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (DragBorder && DragBorder->GetCachedGeometry().IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
		{
			bIsDraggingWindow = true;
			return FReply::Handled().CaptureMouse(TakeWidget());
		}
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UCPShopPurchasePopupWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDraggingWindow && HasMouseCapture())
	{
		float DPIScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
		FVector2D ScaledDelta = InMouseEvent.GetCursorDelta() / DPIScale;
		FVector2D NewTranslation = GetRenderTransform().Translation + ScaledDelta;
		SetRenderTranslation(NewTranslation);
		return FReply::Handled();
	}
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UCPShopPurchasePopupWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDraggingWindow && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDraggingWindow = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UCPShopPurchasePopupWidget::OnPurchaseQuantityChanged(float NewValue)
{
	CurrentQuantity = FMath::RoundToInt(NewValue);
	RefreshDisplay();
}

void UCPShopPurchasePopupWidget::OnQuantityTextCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	// Enter를 누르거나 다른 곳을 클릭해 포커스를 잃었을 때만 반영
	int32 TypedValue = FCString::Atoi(*NewText.ToString());

	int32 MaxStock = QuantitySlider ? FMath::RoundToInt(QuantitySlider->GetMaxValue()) : TypedValue;
	CurrentQuantity = FMath::Clamp(TypedValue, 1, MaxStock);

	if (QuantitySlider)
	{
		QuantitySlider->SetValue(static_cast<float>(CurrentQuantity));
	}

	RefreshDisplay(); // 잘못된 값(범위 밖, 문자 등)을 입력했어도 여기서 Clamp된 값으로 다시 텍스트를 덮어씀
}

void UCPShopPurchasePopupWidget::OpenForPurchase(UCPItemContainerComponent* InShopContainer, int32 InGridIndex, UCPInventoryComponent* InPlayerInventory)
{
	if (!InShopContainer || !InPlayerInventory) return;

	FContainerItem* ShopItem = InShopContainer->ContainerItems.FindByPredicate(
		[InGridIndex](const FContainerItem& It) { return It.GridIndex == InGridIndex; });
	if (!ShopItem || !ShopItem->Instance.SourceItemData) return;

	CurrentMode = EShopPopupMode::Buy;
	ShopContainer = InShopContainer;
	ShopGridIndex = InGridIndex;
	PlayerInventory = InPlayerInventory;
	PlayerContainerForSale = nullptr;
	CurrentQuantity = 1;

	if (ModeTitleText) ModeTitleText->SetText(FText::FromString(TEXT("구매하기")));

	if (QuantitySlider)
	{
		QuantitySlider->SetMinValue(1.f);
		QuantitySlider->SetMaxValue(static_cast<float>(ShopItem->Stacked));		// 현재 재고
		QuantitySlider->SetValue(1.f);
		QuantitySlider->SetStepSize(1.f);
	}

	if (ErrorMessageText) ErrorMessageText->SetText(FText::GetEmpty());

	SetAlignmentInViewport(FVector2D(0.f, 0.f));
	FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
	SetPositionInViewport(MousePos, false); // 마우스 위치를 좌상단으로 

	SetVisibility(ESlateVisibility::Visible);
	RefreshDisplay();
}

void UCPShopPurchasePopupWidget::OpenForSale(UCPItemContainerComponent* InPlayerContainer, int32 InGridIndex, UCPItemContainerComponent* InShopContainer, UCPInventoryComponent* InPlayerInventory)
{
	if (!InPlayerContainer || !InShopContainer || !InPlayerInventory) return;

	FContainerItem* PlayerItem = InPlayerContainer->ContainerItems.FindByPredicate(
		[InGridIndex](const FContainerItem& It) { return It.GridIndex == InGridIndex; });
	if (!PlayerItem || !PlayerItem->Instance.SourceItemData) return;

	CurrentMode = EShopPopupMode::Sell;
	PlayerContainerForSale = InPlayerContainer;
	ShopContainer = InShopContainer;
	ShopGridIndex = InGridIndex;              // 판매 대상 슬롯은 "플레이어 컨테이너 기준" 인덱스로 재사용
	PlayerInventory = InPlayerInventory;
	CurrentQuantity = 1;

	if (ModeTitleText) ModeTitleText->SetText(FText::FromString(TEXT("판매하기")));

	if (QuantitySlider)
	{
		QuantitySlider->SetMinValue(1.f);
		QuantitySlider->SetMaxValue(static_cast<float>(PlayerItem->Stacked)); // 내가 가진 수량이 최대치
		QuantitySlider->SetValue(1.f);
		QuantitySlider->SetStepSize(1.f);
	}

	if (ErrorMessageText) ErrorMessageText->SetText(FText::GetEmpty());

	SetAlignmentInViewport(FVector2D(0.f, 0.f));
	FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
	SetPositionInViewport(MousePos, false);

	SetVisibility(ESlateVisibility::Visible);
	RefreshDisplay();
}

void UCPShopPurchasePopupWidget::OnQuantityChanged(float NewValue)
{
	CurrentQuantity = FMath::RoundToInt(NewValue);
	RefreshDisplay();
}

void UCPShopPurchasePopupWidget::RefreshDisplay()
{
	UCPItemContainerComponent* SourceContainer = (CurrentMode == EShopPopupMode::Buy) ? ShopContainer : PlayerContainerForSale;
	if (!SourceContainer) return;

	FContainerItem* Item = SourceContainer->ContainerItems.FindByPredicate(
		[this](const FContainerItem& It) { return It.GridIndex == ShopGridIndex; });
	if (!Item || !Item->Instance.SourceItemData) return;

	UCPForageableItemData* Data = Item->Instance.SourceItemData;

	if (ItemIcon)
	{
		if (UTexture2D* LoadedTexture = Data->Icon.LoadSynchronous())
		{
			ItemIcon->SetBrushFromTexture(LoadedTexture);
		}
	}

	if (QuantityInputBox)
	{
		QuantityInputBox->SetText(FText::AsNumber(CurrentQuantity));
	}

	int32 UnitPrice = (CurrentMode == EShopPopupMode::Buy) ? Data->BuyPrice : Data->GetSellPrice();

	if (TotalPriceText)
	{
		TotalPriceText->SetText(FText::AsNumber(UnitPrice * CurrentQuantity));
	}
}

void UCPShopPurchasePopupWidget::OnConfirmClicked()
{
	if (CurrentMode == EShopPopupMode::Buy)
	{
		if (!ShopContainer || !PlayerInventory) return;

		FContainerItem* ShopItem = ShopContainer->ContainerItems.FindByPredicate(
			[this](const FContainerItem& It) { return It.GridIndex == ShopGridIndex; });
		if (!ShopItem || !ShopItem->Instance.SourceItemData) return;

		int32 TotalPrice = ShopItem->Instance.SourceItemData->BuyPrice * CurrentQuantity;

		if (!PlayerInventory->TrySpendMoney(TotalPrice))
		{
			if (ErrorMessageText)
			{
				ErrorMessageText->SetText(FText::FromString(TEXT("소지 금액이 부족합니다.")));
			}

			return;
		}

		FCPItemInstance PurchasedInstance = ShopItem->Instance;
		ShopContainer->RemoveItemFromContainer(ShopGridIndex, CurrentQuantity);
		PlayerInventory->TryGetItemFromInstance(PurchasedInstance, CurrentQuantity);
	}
	else // Sell
	{
		if (!PlayerContainerForSale || !ShopContainer || !PlayerInventory) return;

		FContainerItem* PlayerItem = PlayerContainerForSale->ContainerItems.FindByPredicate(
			[this](const FContainerItem& It) { return It.GridIndex == ShopGridIndex; });
		if (!PlayerItem || !PlayerItem->Instance.SourceItemData) return;

		int32 SellPricePerUnit = PlayerItem->Instance.SourceItemData->GetSellPrice();
		FCPItemInstance InstanceCopy = PlayerItem->Instance;

		int32 Leftover = ShopContainer->TryGetItemFromInstance(InstanceCopy, CurrentQuantity);
		int32 ActuallySold = CurrentQuantity - Leftover;

		if (ActuallySold <= 0)
		{
			if (ErrorMessageText)
			{
				ErrorMessageText->SetText(FText::FromString(TEXT("상점 재고 공간이 부족합니다.")));
			}
			return;
		}

		PlayerContainerForSale->RemoveItemFromContainer(ShopGridIndex, ActuallySold);
		PlayerInventory->TryGetMoney(SellPricePerUnit * ActuallySold);
	}

	ClosePopup();
}

void UCPShopPurchasePopupWidget::OnCancelClicked()
{
	ClosePopup();
}

void UCPShopPurchasePopupWidget::ClosePopup()
{
	SetVisibility(ESlateVisibility::Collapsed);
	ShopContainer = nullptr;
	PlayerInventory = nullptr;
	ShopGridIndex = -1;
}