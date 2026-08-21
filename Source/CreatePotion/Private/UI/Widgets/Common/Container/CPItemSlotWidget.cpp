// CPItemSlotWidget.h

#include "UI/Widgets/Common/Container/CPItemSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameFramework/InputSettings.h"			// 더블 클릭 입력 지연 시간을 사용하기 위해

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
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(DoubleClickCheckHandler);
	}
	Super::NativeDestruct();
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

/*
FReply UCPItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	ACPPlayerController* PC = Cast<ACPPlayerController>(GetOwningPlayer());
	bool bIsHolding = PC && (PC->LeftClickPickedContainer != nullptr);

	// 빈 칸이고 "현재 무언가를 들고 있지 않으면(bIsHolding값이 false일 때)" 기존대로 엔진 자체 처리
	if ((!CachedItemData.ItemDataAsset && !bIsHolding) || !OwnerContainer)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	} // 빈 칸이라도 "현재 무언가를 들고 있으면(bIsHolding값이 true일 때)" 마우스 입력 처리를 하러 다음 코드 블럭으로 이동

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
		else
		{
			// 일반 좌클릭을 즉시 실행하지 않고 타이머 대기 (더블 클릭 판별용)
			float DoubleClickDelay = GetDefault<UInputSettings>()->DoubleClickTime;	// 엔진의 더블클릭 판정 시간
			GetWorld()->GetTimerManager().SetTimer(
				DoubleClickCheckHandler, 
				this, 
				&UCPItemSlotWidget::ExecuteLeftClickOnce, 
				DoubleClickDelay,
				false);
			bHandled = true; // 일단 어떤 경우던 처리될 예정이니 Handled 처리
		}
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
*/

void UCPItemSlotWidget::ExecuteLeftClickOnce()
{
	if (ACPPlayerController* PC = Cast<ACPPlayerController>(GetOwningPlayer()))
	{
		if (PC->CurrentContextHandler)
		{
			PC->CurrentContextHandler->HandleLeftClickOnly(this);
		}
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
			World->GetTimerManager().ClearTimer(DoubleClickCheckHandler);
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
