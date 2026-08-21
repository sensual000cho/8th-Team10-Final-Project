// CPItemSlotWidget.h

#include "UI/Widgets/Common/Container/CPItemSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "CreatePotion.h"   // 로그용 헤더
#include "Data/CPForageableItemData.h"
#include "Components/CPItemContainerComponent.h"	// CPContainerItem 구조체
#include "Character/CPPlayerController.h"			// 상호작용하는 대상의 컨테이너를 가지고 있음
#include "UI/Context/CPContainerContextBase.h"		// Context 분기를 처리할 클래스

void UCPItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 마우스 클릭 이벤트를 이 클래스가 처리할 수 있도록 Visible로 설정
	SetVisibility(ESlateVisibility::Visible);

	if (ItemIcon)
	{
		ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (StackText)
	{
		StackText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	ClearSlot();
}

void UCPItemSlotWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UCPItemSlotWidget::UpdateSlot(const FContainerItem& ItemData)
{
	CachedItemData = ItemData; // FContainerItem 구조체를 전달받으면 캐싱
	this->SlotGridIndex = ItemData.GridIndex;	// 아이템의 GridIndex로 업데이트

	// 유효한 아이템 데이터가 있는지 확인
	if (ItemData.ItemDataAsset)
	{
		// 아이콘 텍스쳐
		UTexture2D* LoadedTexture = ItemData.ItemDataAsset->Icon.LoadSynchronous();
		if (LoadedTexture)
		{
			ItemIcon->SetBrushFromTexture(LoadedTexture);
		}

		// SelfHitTestInvisible은 이미지는 보이되, 마우스 클릭은 통과시키라는 설정
		ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		// 스택 텍스트
		if (ItemData.Stacked > 1)
		{
			StackText->SetText(FText::AsNumber(ItemData.Stacked));
			StackText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			// 1개일 때는 수량을 표시하지 않도록 숨기기
			StackText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		ClearSlot();	// 데이터가 없으면 빈 칸 처리
	}
}

void UCPItemSlotWidget::ClearSlot()
{
	CachedItemData = FContainerItem();	// 기본 생성자 호출을 통해 빈 구조체로 초기화
	this->SlotGridIndex = -1;		// Slot의 index도 초기화

	// 아이콘과 텍스트를 모두 Collapsed 처리
	if (ItemIcon)
	{
		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (StackText)
	{
		StackText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

FReply UCPItemSlotWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			// 더블 클릭이 들어왔으므로, 대기 중이던 일반(한 번) 좌클릭 타이머를 즉시 취소
			CancelLeftClickCheckHandler();
		}

		if (CachedItemData.ItemDataAsset && OwnerContainer)
		{
			if (ACPPlayerController* PC = Cast<ACPPlayerController>(GetOwningPlayer()))
			{
				if (PC->CurrentContextHandler)
				{
					bool bHandled = PC->CurrentContextHandler->HandleLeftDoubleClick(this);
					return bHandled ? FReply::Handled() : Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
				}
			}
		}
	}

	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}
