// CPItemSlotWidget.h

#include "UI/Widgets/Common/Container/CPItemSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "CreatePotion.h"   // 로그용 헤더
#include "Data/CPForageableItemData.h"
#include "Components/CPItemContainerComponent.h"	// CPContainerItem 구조체

void UCPItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ClearSlot();
}

void UCPItemSlotWidget::UpdateSlot(const FContainerItem& ItemData)
{
	CachedItemData = ItemData; // FContainerItem 구조체를 전달받으면 캐싱

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
