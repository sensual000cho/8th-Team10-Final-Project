// CPItemDetailPopupWidget.cpp

#include "UI/Widgets/Common/Container/CPItemDetailPopupWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Types/CPItemInstanceTypes.h"
#include "Data/CPForageableItemData.h"
#include "Data/CPTagDefinitionTypes.h"
#include "Blueprint/WidgetLayoutLibrary.h"

#include "UI/Widgets/Base/CPWidgetLocationPlacer.h"

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

	if (TagText)
	{
		const bool bIsPotion = Instance.CurrentEffects.Num() > 0;
		TArray<FGameplayTag> Tags = bIsPotion ? TArray<FGameplayTag>() : Instance.GetDisplayTags();

		if (Tags.Num() == 0)
		{
			TagText->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			TArray<FText> TagNames;
			for (const FGameplayTag& Tag : Tags)
			{
				TagNames.Add(FCPTagDefinitionLookup::GetDisplayName(Tag));
			}

			TagText->SetText(FText::Format(
				FText::FromString(TEXT("태그 : {0}")),
				FText::Join(FText::FromString(TEXT(", ")), TagNames)));
			TagText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
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
		if (Instance.IsPotion())
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
	const FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
	const FVector2D Pos = CPWidgetLocationPlacer::CalcPositionNearCursor(this, MousePos, FVector2D(20.f, 20.f));
	SetPositionInViewport(Pos, false);
}

void UCPItemDetailPopupWidget::HideDetail()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
