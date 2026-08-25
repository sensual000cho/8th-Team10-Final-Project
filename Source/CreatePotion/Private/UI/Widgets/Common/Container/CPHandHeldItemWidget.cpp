// CPHandHeldItemWidget.cpp

#include "UI/Widgets/Common/Container/CPHandHeldItemWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"

#include "UI/Widgets/Common/Container/CPContainerGridWidget.h"
#include "Components/CPItemContainerComponent.h"
#include "Data/CPForageableItemData.h"
#include "Settings/CPContainerUISettings.h"

void UCPHandHeldItemWidget::TryBindHandContainer(UCPItemContainerComponent* InHandContainer)
{
	if (!InHandContainer)
	{
		return;
	}

	HandContainer = InHandContainer;
	HandContainer->OnContainerUpdated.AddDynamic(this, &UCPHandHeldItemWidget::OnHandContainerUpdated);

	SetAlignmentInViewport(FVector2D(0.f, 0.f));

	OnHandContainerUpdated(); // 최초 업데이트 (= Collapsed로)
}

void UCPHandHeldItemWidget::OnHandContainerUpdated()
{
	if (!HandContainer || HandContainer->ContainerItems.Num() == 0)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const FContainerItem& Held = HandContainer->ContainerItems[0];
	if (!Held.ItemDataAsset)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ESlateVisibility::HitTestInvisible); // 커서를 따라다니되 마우스 클릭은 통과
	UpdatePositionToMouse();

	// 회전 여부까지 반영한 실제 가로/세로 칸 수
	int32 ItemW = Held.bIsRotated ? Held.ItemDataAsset->ContainerSizeY : Held.ItemDataAsset->ContainerSizeX;
	int32 ItemH = Held.bIsRotated ? Held.ItemDataAsset->ContainerSizeX : Held.ItemDataAsset->ContainerSizeY;

	if (RootSizeBox)
	{
		const UCPContainerUISettings* Settings = GetDefault<UCPContainerUISettings>();

		float SizeX = (ItemW * Settings->SlotSize) + FMath::Max(0, (ItemW - 1)) * Settings->SlotPadding;
		float SizeY = (ItemH * Settings->SlotSize) + FMath::Max(0, (ItemH - 1)) * Settings->SlotPadding;
		RootSizeBox->SetWidthOverride(SizeX);
		RootSizeBox->SetHeightOverride(SizeY);
	}

	UTexture2D* LoadedTexture = Held.ItemDataAsset->Icon.LoadSynchronous();
	if (LoadedTexture && ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(LoadedTexture);
	}

	if (StackText)
	{
		if (Held.Stacked > 1)
		{
			StackText->SetText(FText::AsNumber(Held.Stacked));
			StackText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			StackText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UCPHandHeldItemWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdatePositionToMouse();
}

void UCPHandHeldItemWidget::UpdatePositionToMouse()
{
	FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
	float HalfCell = CalcCellCenterOffset();
	FVector2D AdjustedPos = MousePos - FVector2D(HalfCell, HalfCell);
	SetPositionInViewport(AdjustedPos, false);
}

float UCPHandHeldItemWidget::CalcCellCenterOffset() const
{
	const UCPContainerUISettings* Settings = GetDefault<UCPContainerUISettings>();
	return Settings->SlotSize * 0.5f;
}
