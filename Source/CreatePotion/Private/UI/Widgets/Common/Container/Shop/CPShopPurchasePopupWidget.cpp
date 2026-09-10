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

	ShopContainer = InShopContainer;
	ShopGridIndex = InGridIndex;
	PlayerInventory = InPlayerInventory;
	CurrentQuantity = 1;

	if (QuantitySlider)
	{
		QuantitySlider->SetMinValue(1.f);
		QuantitySlider->SetMaxValue(static_cast<float>(ShopItem->Stacked));		// 현재 재고
		QuantitySlider->SetValue(1.f);
		QuantitySlider->SetStepSize(1.f);
	}

	if (ErrorMessageText) ErrorMessageText->SetText(FText::GetEmpty());

	// 클릭 위치를 위젯의 좌상단으로 삼도록 정렬 고정
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
	if (!ShopContainer)
	{
		return;
	}

	FContainerItem* ShopItem = ShopContainer->ContainerItems.FindByPredicate(
		[this](const FContainerItem& It) { return It.GridIndex == ShopGridIndex; });
	if (!ShopItem || !ShopItem->Instance.SourceItemData)
	{
		return;
	}

	UCPForageableItemData* Data = ShopItem->Instance.SourceItemData;

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

	if (TotalPriceText)
	{
		TotalPriceText->SetText(FText::AsNumber(Data->BuyPrice * CurrentQuantity));
	}
}

void UCPShopPurchasePopupWidget::OnConfirmClicked()
{
	if (!ShopContainer || !PlayerInventory)
	{
		return;
	}

	FContainerItem* ShopItem = ShopContainer->ContainerItems.FindByPredicate(
		[this](const FContainerItem& It) { return It.GridIndex == ShopGridIndex; });
	if (!ShopItem || !ShopItem->Instance.SourceItemData)
	{
		return;
	}

	UCPForageableItemData* Data = ShopItem->Instance.SourceItemData;
	int32 TotalPrice = Data->BuyPrice * CurrentQuantity;

	if (!PlayerInventory->TrySpendMoney(TotalPrice))
	{
		if (ErrorMessageText)
		{
			ErrorMessageText->SetText(FText::FromString(TEXT("소지 금액이 부족합니다.")));
		}
		return; // 다시 시도할 수 있도록 팝업은 닫지 않고 그대로 유지
	}

	// 구매 전 인스턴스를 값으로 복사해둠 (RemoveItemFromContainer가 배열을 바꿀 수 있으므로)
	FCPItemInstance PurchasedInstance = ShopItem->Instance;

	ShopContainer->RemoveItemFromContainer(ShopGridIndex, CurrentQuantity); // 상점 재고 차감
	PlayerInventory->TryGetItemFromInstance(PurchasedInstance, CurrentQuantity); // 플레이어에게 지급

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