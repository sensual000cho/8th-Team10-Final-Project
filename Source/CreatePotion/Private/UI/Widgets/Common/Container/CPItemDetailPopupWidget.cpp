// CPItemDetailPopupWidget.cpp

#include "UI/Widgets/Common/Container/CPItemDetailPopupWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Types/CPItemInstanceTypes.h"
#include "Data/CPForageableItemData.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UCPItemDetailPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCPItemDetailPopupWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (GetVisibility() != ESlateVisibility::Collapsed)
	{
		UpdatePositionToMouse();
	}
}

void UCPItemDetailPopupWidget::ShowDetail(const FCPItemInstance& Instance)
{
	if (!Instance.IsValid())
	{
		return;
	}

	if (NameText)
	{
		NameText->SetText(Instance.GetDisplayName());
	}

	if (ItemIcon)
	{
		if (UTexture2D* LoadedTexture = Instance.SourceItemData->Icon.LoadSynchronous())
		{
			ItemIcon->SetBrushFromTexture(LoadedTexture);
		}
	}

	// 포션(효과 태그가 있는 경우)일 때만 액체 레이어 표시
	if (LiquidImage)
	{
		if (Instance.CurrentEffects.Num() > 0)
		{
			LiquidImage->SetColorAndOpacity(Instance.GetLiquidColor());
			LiquidImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			LiquidImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	SetAlignmentInViewport(FVector2D(0.f, 0.f));
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	UpdatePositionToMouse();
}

void UCPItemDetailPopupWidget::UpdatePositionToMouse()
{
	FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
	SetPositionInViewport(MousePos + FVector2D(20.f, 20.f), false);
}

void UCPItemDetailPopupWidget::HideDetail()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
