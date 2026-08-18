// CPContainerGridWidget.cpp

#include "UI/Widgets/Common/Container/CPContainerGridWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"		// 사이즈 조절

#include "CreatePotion.h"   // 로그용 헤더
#include "UI/Widgets/Common/Container/CPItemSlotWidget.h"
#include "Components/CPItemContainerComponent.h"

void UCPContainerGridWidget::InitializeGrid(UCPItemContainerComponent* TargetContainer)
{
	if (!TargetContainer || !BackgroundSlotClass || !ItemGrid)
	{
		return;
	}

	// UpdateGrid에서 Columns 값을 쓰기 위해 캐싱
	CachedContainer = TargetContainer;

	ItemGrid->ClearChildren();

	int32 TotalSlots = 0;

	if (TargetContainer->ContainerType == EContainerType::Grid2D)
	{
		TotalSlots = TargetContainer->Columns * TargetContainer->Rows;
	}
	else if (TargetContainer->ContainerType == EContainerType::Slot1D)
	{
		TotalSlots = TargetContainer->MaxSlots;
	}

	// 텅 빈 배경 Grid를 Column, Row에 맞춰 배치
	for (int32 i = 0; i < TotalSlots; ++i)
	{
		UUserWidget* BgSlot = CreateWidget<UUserWidget>(this, BackgroundSlotClass);
		if (BgSlot)
		{
			int32 Row = 0;
			int32 Col = i;

			if (TargetContainer->ContainerType == EContainerType::Grid2D)
			{
				Row = i / TargetContainer->Columns;
				Col = i % TargetContainer->Columns;

				// Grid2D의 경우 UniformGrid에 바로 추가
				ItemGrid->AddChildToUniformGrid(BgSlot, Row, Col);
			}
			else if (TargetContainer->ContainerType == EContainerType::Slot1D)
			{
				// Slot1D의 경우 1칸짜리 배경을 (Columns x Rows) 크기로 거대하게 그리기
				float BigSlotSizeX = (TargetContainer->Columns * SlotSize) + FMath::Max(0, (TargetContainer->Columns - 1)) * SlotPadding;
				float BigSlotSizeY = (TargetContainer->Rows * SlotSize) + FMath::Max(0, (TargetContainer->Rows - 1)) * SlotPadding;

				// SizeBox를 동적으로 생성해서 배경 위젯을 감싼 뒤 강제로 크기를 늘림
				USizeBox* SizeBox = NewObject<USizeBox>(this);
				SizeBox->SetWidthOverride(BigSlotSizeX);
				SizeBox->SetHeightOverride(BigSlotSizeY);
				SizeBox->AddChild(BgSlot);

				// 확장된 SizeBox를 UniformGrid에 추가 (가로로 일렬 배치)
				ItemGrid->AddChildToUniformGrid(SizeBox, Row, Col);
			}
		}
	}
}

void UCPContainerGridWidget::UpdateGrid(const TArray<FContainerItem>& ContainerItems)
{
	if (!ItemWidgetClass || !ItemCanvas || !CachedContainer)
	{
		return;
	}

	// 기존에 캔버스에 그려진 아이템 위젯들을 모두 지우기
	ItemCanvas->ClearChildren();

	for (const FContainerItem& Item : ContainerItems)
	{
		// 데이터가 없는 빈 Grid는 무시, 반복문 바로 종료 후 다음 Grid로 이동
		if (!Item.ItemDataAsset || Item.GridIndex < 0)
		{
			continue;
		}

		// 새로운 아이템 위젯을 생성
		UCPItemSlotWidget* NewItemWidget = CreateWidget<UCPItemSlotWidget>(this, ItemWidgetClass);
		if (!NewItemWidget)
		{
			continue;
		}

		// 캔버스 패널에 먼저 추가 (NativeConstruct가 실행되며 ClearSlot이 호출됨)
		UCanvasPanelSlot* CanvasSlot = ItemCanvas->AddChildToCanvas(NewItemWidget);
		if (CanvasSlot)
		{
			float PosX = 0.0f;
			float PosY = 0.0f;

			// 아이템의 실제 크기 (패딩 포함)
			int32 ItemW = Item.bIsRotated ? Item.ItemDataAsset->ContainerSizeY : Item.ItemDataAsset->ContainerSizeX;
			int32 ItemH = Item.bIsRotated ? Item.ItemDataAsset->ContainerSizeX : Item.ItemDataAsset->ContainerSizeY;

			float ItemSizeX = (ItemW * SlotSize) + FMath::Max(0, (ItemW - 1)) * SlotPadding;
			float ItemSizeY = (ItemH * SlotSize) + FMath::Max(0, (ItemH - 1)) * SlotPadding;

			if (CachedContainer->ContainerType == EContainerType::Slot1D)
			{
				// 배경(BigSlot)의 물리적 크기
				float BigSlotSizeX = (CachedContainer->Columns * SlotSize) + FMath::Max(0, (CachedContainer->Columns - 1)) * SlotPadding;
				float BigSlotSizeY = (CachedContainer->Rows * SlotSize) + FMath::Max(0, (CachedContainer->Rows - 1)) * SlotPadding;

				// 다음 거대 슬롯으로 넘어갈 때의 간격: (거대 슬롯 크기 + 패딩 1개)
				float BasePosX = Item.GridIndex * (BigSlotSizeX + SlotPadding);
				float BasePosY = 0.0f;

				// 중앙 정렬
				PosX = BasePosX + ((BigSlotSizeX - ItemSizeX) / 2.0f);
				PosY = BasePosY + ((BigSlotSizeY - ItemSizeY) / 2.0f);
			}
			else
			{
				// 기존 Grid2D 로직
				int32 Col = Item.GridIndex % CachedContainer->Columns;
				int32 Row = Item.GridIndex / CachedContainer->Columns;

				PosX = Col * (SlotSize + SlotPadding);
				PosY = Row * (SlotSize + SlotPadding);
			}

			// 캔버스 상의 위치와 크기 적용
			CanvasSlot->SetAutoSize(false);
			CanvasSlot->SetPosition(FVector2D(PosX, PosY));
			CanvasSlot->SetSize(FVector2D(ItemSizeX, ItemSizeY));

			NewItemWidget->UpdateSlot(Item);
			NewItemWidget->OwnerContainer = CachedContainer;
			NewItemWidget->CachedItemData = Item;
		}
	}
}
