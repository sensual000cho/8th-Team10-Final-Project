// CPItemWheelWidget.cpp

#include "UI/Widgets/Common/Container/CPItemWheelWidget.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"

#include "CreatePotion.h"
#include "Components/CPInventoryComponent.h"
#include "Data/CPForageableItemData.h"

void UCPItemWheelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 루트는 항상 Visible로 유지 (= 휠 입력을 계속 감지)
	SetVisibility(ESlateVisibility::Visible);

	SetSlotVisible(Left2Icon, Left2Liquid, false);
	SetSlotVisible(Left1Icon, Left1Liquid, false);
	SetSlotVisible(FocusIcon, FocusLiquid, false);
	SetSlotVisible(Right1Icon, Right1Liquid, false);
	SetSlotVisible(Right2Icon, Right2Liquid, false);
}

void UCPItemWheelWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);
	Super::NativeDestruct();
}

void UCPItemWheelWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (PendingDirection == 0 || !IconRow)
	{
		return;
	}

	SlideProgress += InDeltaTime * SlideSpeed;

	if (SlideProgress >= 1.0f)
	{
		int32 Count = PotionArrayIndices.Num();
		FocusedListIndex = (FocusedListIndex + PendingDirection + Count) % Count;

		PendingDirection = 0;
		SlideProgress = 0.f;
		IconRow->SetRenderTranslation(FVector2D::ZeroVector);

		RefreshIcons();
	}
	else
	{
		float CurrentOffsetX = -PendingDirection * SlotStride * SlideProgress;
		IconRow->SetRenderTranslation(FVector2D(CurrentOffsetX, 0.f));

		UpdateSlotScales(SlideProgress);
	}
}

void UCPItemWheelWidget::BindInventory(UCPInventoryComponent* InInventory)
{
	if (!InInventory || BoundInventory == InInventory)
	{
		return;
	}

	BoundInventory = InInventory;
	BoundInventory->OnContainerUpdated.AddDynamic(this, &UCPItemWheelWidget::OnInventoryUpdated);

	RefreshPotionList();
}

void UCPItemWheelWidget::OnInventoryUpdated()
{
	RefreshPotionList();

	// 지금 아이콘이 보이는 중(=휠 조작 중)이었다면 최신 목록으로 다시 그려줌
	if (FocusIcon && FocusIcon->GetVisibility() != ESlateVisibility::Collapsed)
	{
		RefreshIcons();
	}
}

void UCPItemWheelWidget::RefreshPotionList()
{
	PotionArrayIndices.Empty();

	if (!BoundInventory)
	{
		return;
	}

	for (int32 i = 0; i < BoundInventory->ContainerItems.Num(); ++i)
	{
		const FContainerItem& Item = BoundInventory->ContainerItems[i];
		if (Item.Instance.SourceItemData && Item.Instance.CurrentEffects.Num() > 0)
		{
			PotionArrayIndices.Add(i);
		}
	}

	// 목록이 줄어들어 포커스 인덱스가 범위를 벗어난 경우 보정
	if (PotionArrayIndices.Num() > 0)
	{
		FocusedListIndex = FMath::Clamp(FocusedListIndex, 0, PotionArrayIndices.Num() - 1);
	}
	else
	{
		FocusedListIndex = 0;
	}

	UE_LOG(LogContainer, Warning, TEXT("[ItemWheel] 포션 목록 갱신, 개수: %d"), PotionArrayIndices.Num());
}

void UCPItemWheelWidget::RefreshIcons()
{
	if (!BoundInventory || PotionArrayIndices.Num() == 0)
	{
		SetSlotVisible(Left2Icon, Left2Liquid, false);
		SetSlotVisible(Left1Icon, Left1Liquid, false);
		SetSlotVisible(FocusIcon, FocusLiquid, false);
		SetSlotVisible(Right1Icon, Right1Liquid, false);
		SetSlotVisible(Right2Icon, Right2Liquid, false);
		return;
	}

	int32 Count = PotionArrayIndices.Num();

	auto SetIcon = [this, Count](UImage* IconWidget, UImage* LiquidWidget, int32 Offset, float Alpha)
		{
			int32 Idx = (FocusedListIndex + Offset + Count * 10) % Count;		// 음수 방지
			const FContainerItem& Item = BoundInventory->ContainerItems[PotionArrayIndices[Idx]];

			if (UTexture2D* LoadedTexture = Item.Instance.SourceItemData->Icon.LoadSynchronous())
			{
				IconWidget->SetBrushFromTexture(LoadedTexture);
			}
			IconWidget->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Alpha));
			IconWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

			float FinalScale = GetTargetScaleForOffset(Offset);
			IconWidget->SetRenderScale(FVector2D(FinalScale, FinalScale));

			if (LiquidWidget)
			{
				FLinearColor LiquidColor = Item.Instance.GetLiquidColor();
				LiquidColor.A = Alpha;
				LiquidWidget->SetColorAndOpacity(LiquidColor);
				LiquidWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
				LiquidWidget->SetRenderScale(FVector2D(FinalScale, FinalScale));
			}
		};

	SetIcon(FocusIcon, FocusLiquid, 0, 1.0f);

	if (Count > 1)
	{
		SetIcon(Left1Icon, Left1Liquid, -1, NearIconAlpha);
		SetIcon(Right1Icon, Right1Liquid, 1, NearIconAlpha);
	}
	else
	{
		SetSlotVisible(Left1Icon, Left1Liquid, false);
		SetSlotVisible(Right1Icon, Right1Liquid, false);
	}

	if (Count > 3)
	{
		SetIcon(Left2Icon, Left2Liquid, -2, FarIconAlpha);
		SetIcon(Right2Icon, Right2Liquid, 2, FarIconAlpha);
	}
	else
	{
		SetSlotVisible(Left2Icon, Left2Liquid, false);
		SetSlotVisible(Right2Icon, Right2Liquid, false);
	}
}

void UCPItemWheelWidget::ShowWheelTemporarily()
{
	GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);

	GetWorld()->GetTimerManager().SetTimer(
		HideTimerHandle,
		this,
		&UCPItemWheelWidget::HideWheel,
		AutoHideDelay,
		false
	);
}

void UCPItemWheelWidget::HideWheel()
{
	SetSlotVisible(Left2Icon, Left2Liquid, false);
	SetSlotVisible(Left1Icon, Left1Liquid, false);
	SetSlotVisible(FocusIcon, FocusLiquid, false);
	SetSlotVisible(Right1Icon, Right1Liquid, false);
	SetSlotVisible(Right2Icon, Right2Liquid, false);
}

void UCPItemWheelWidget::SetSlotVisible(UImage* IconWidget, UImage* LiquidWidget, bool bVisible)
{
	ESlateVisibility NewVis = bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
	if (IconWidget) IconWidget->SetVisibility(NewVis);
	if (LiquidWidget) LiquidWidget->SetVisibility(NewVis);
}

void UCPItemWheelWidget::UpdateSlotScales(float Progress)
{
	auto ApplyScale = [this, Progress](UImage* IconWidget, UImage* LiquidWidget, int32 CurrentOffset)
		{
			int32 NextOffset = CurrentOffset - PendingDirection;
			float FromScale = GetTargetScaleForOffset(CurrentOffset);
			float ToScale = GetTargetScaleForOffset(NextOffset);
			float Scale = FMath::Lerp(FromScale, ToScale, Progress);

			if (IconWidget) IconWidget->SetRenderScale(FVector2D(Scale, Scale));
			if (LiquidWidget) LiquidWidget->SetRenderScale(FVector2D(Scale, Scale));
		};

	ApplyScale(Left2Icon, Left2Liquid, -2);
	ApplyScale(Left1Icon, Left1Liquid, -1);
	ApplyScale(FocusIcon, FocusLiquid, 0);
	ApplyScale(Right1Icon, Right1Liquid, 1);
	ApplyScale(Right2Icon, Right2Liquid, 2);
}

float UCPItemWheelWidget::GetTargetScaleForOffset(int32 Offset) const
{
	switch (Offset)
	{
	case 0:  
		return FocusScale;
	case -1:
	case 1:  
		return NearScale;
	default: 
		return FarScale;
	}
}

bool UCPItemWheelWidget::GetFocusedPotionGridIndex(int32& OutGridIndex) const
{
	if (!BoundInventory || PotionArrayIndices.Num() == 0)
	{
		return false;
	}

	int32 ArrayIdx = PotionArrayIndices[FocusedListIndex];
	OutGridIndex = BoundInventory->ContainerItems[ArrayIdx].GridIndex;
	return true;
}

void UCPItemWheelWidget::HandleScrollInput(float ScrollDelta)
{
	if (PotionArrayIndices.Num() == 0 || PendingDirection != 0)
	{
		return; // 이미 슬라이드 애니메이션 진행 중이면 새 입력 무시 (연타 방지)
	}

	PendingDirection = (ScrollDelta > 0.f) ? 1 : -1;
	SlideProgress = 0.f;

	ShowWheelTemporarily();
}