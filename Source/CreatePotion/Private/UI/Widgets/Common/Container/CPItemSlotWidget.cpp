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

FReply UCPItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 빈 칸(유효한 데이터가 없음)이면 무시하고 부모 함수로 처리
	// TODO[Container] : 아이템을 드롭할 때 빈 칸에 상호작용을 해야하기 때문에 수정해야할 부분
	if (!CachedItemData.ItemDataAsset || !OwnerContainer)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	ACPPlayerController* PC = Cast<ACPPlayerController>(GetOwningPlayer());
	if (!PC || !PC->CurrentContextHandler)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	UCPContainerContextBase* ContextHandler = PC->CurrentContextHandler;
	if (!ContextHandler)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}
	bool bHandled = false;

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)	// 좌클릭인 경우
	{
		if (InMouseEvent.IsLeftControlDown())      bHandled = ContextHandler->HandleCtrlLeftClick(this);
		else if (InMouseEvent.IsShiftDown())       bHandled = ContextHandler->HandleShiftLeftClick(this);
		else if (InMouseEvent.IsAltDown())         bHandled = ContextHandler->HandleAltLeftClick(this);
		else                                       bHandled = ContextHandler->HandleLeftClickOnly(this);
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)	// 우클릭인 경우
	{
		if (InMouseEvent.IsLeftControlDown())      bHandled = ContextHandler->HandleCtrlRightClick(this);
		else if (InMouseEvent.IsShiftDown())       bHandled = ContextHandler->HandleShiftRightClick(this);
		else if (InMouseEvent.IsAltDown())         bHandled = ContextHandler->HandleAltRightClick(this);
		else                                       bHandled = ContextHandler->HandleRightClickOnly(this);
	}

	// bHandled는 Context로 처리했는지 여부
	// Context로 처리 했다면 FReply::Handled() 처리, 이벤트를 처리했음을 엔진에 알림
	// 마우스 클릭 이벤트가 마무리 되며 뒤 UI는 클릭 이벤트가 실행되지 않음
	return bHandled ? FReply::Handled() : Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
