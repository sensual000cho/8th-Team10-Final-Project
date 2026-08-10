// CPContainerGridWidget.cpp

#include "UI/Widgets/Common/Container/CPContainerGridWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

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
			}

			ItemGrid->AddChildToUniformGrid(BgSlot, Row, Col);
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

		// 캔버스 패널에 먼저 추가
		// NativeConstruct가 실행되며 ClearSlot이 호출됨
		UCanvasPanelSlot* CanvasSlot = ItemCanvas->AddChildToCanvas(NewItemWidget);

		if (CanvasSlot)
		{
			// 캐싱해둔 Container의 Column
			int32 Columns = CachedContainer->Columns;

			// 만약 장비창 같은 Slot1D라면 가로로 길게 배치되도록 계산
			if (CachedContainer->ContainerType == EContainerType::Slot1D)
			{
				Columns = CachedContainer->MaxSlots;
			}

			// index를 기반으로 현재 아이템의 Column, Row 위치 계산
			int32 Col = Item.GridIndex % Columns;
			int32 Row = Item.GridIndex / Columns;

			// 아이템의 가로/세로 칸 수 (회전 고려)
			int32 ItemW = Item.bIsRotated ? Item.ItemDataAsset->ContainerSizeY : Item.ItemDataAsset->ContainerSizeX;
			int32 ItemH = Item.bIsRotated ? Item.ItemDataAsset->ContainerSizeX : Item.ItemDataAsset->ContainerSizeY;

			// 위치(Position) 계산: (슬롯 크기 + 여백) * 인덱스
			float PosX = Col * (SlotSize + SlotPadding);
			float PosY = Row * (SlotSize + SlotPadding);

			// 크기(Size) 계산: (차지하는 칸 수 * 슬롯 크기) + (칸 사이사이의 여백 개수 * 여백 크기)
			float SizeX = (ItemW * SlotSize) + ((ItemW - 1) * SlotPadding);
			float SizeY = (ItemH * SlotSize) + ((ItemH - 1) * SlotPadding);

			// 캔버스 상의 위치와 크기를 적용
			CanvasSlot->SetAutoSize(false);
			CanvasSlot->SetPosition(FVector2D(PosX, PosY));
			CanvasSlot->SetSize(FVector2D(SizeX, SizeY));

			NewItemWidget->UpdateSlot(Item); // 위치와 크기까지 캔버스를 확정 후 마지막으로 이미지와 텍스트 업데이트
			NewItemWidget->OwnerContainer = CachedContainer;
			NewItemWidget->CachedItemData = Item;
		}
	}
}
