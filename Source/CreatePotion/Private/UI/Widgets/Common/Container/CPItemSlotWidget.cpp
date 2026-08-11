// CPItemSlotWidget.h

#include "UI/Widgets/Common/Container/CPItemSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "CreatePotion.h"   // 로그용 헤더
#include "Data/CPForageableItemData.h"
#include "Components/CPItemContainerComponent.h"	// CPContainerItem 구조체
#include "Character/CPPlayerController.h"		// 상호작용하는 대상의 컨테이너를 가지고 있음

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
	ACPPlayerController* PC = Cast<ACPPlayerController>(GetOwningPlayer());

	// 빈 칸(유효한 데이터가 없음)이면 무시하고 부모 함수로 처리
	if (!CachedItemData.ItemDataAsset || !OwnerContainer)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	// 좌클릭인 경우
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (InMouseEvent.IsLeftControlDown())
		{
			// TODO: [Ctrl + 좌클릭] 아이템 빠른 이동 (Quick Transfer)
			if (PC && PC->CurrentInteractingContainer)
			{
				UE_LOG(LogContainer, Log, TEXT("[%s] 아이템 이동"), 
					*CachedItemData.ItemDataAsset->DisplayName.ToString());
				OwnerContainer->MoveItemToTargetContainer(CachedItemData.GridIndex, PC->CurrentInteractingContainer);
			}
			else
			{
				UE_LOG(LogContainer, Warning, TEXT("LCtrl : 아이템을 이동시킬 타겟 컨테이너가 없음"));
			}
			return FReply::Handled();	// 이벤트를 처리했음을 엔진에 알림, 
			// 마우스 클릭 이벤트가 마무리 되며 뒤 UI는 클릭 이벤트가 실행되지 않음
		}
		else if (InMouseEvent.IsShiftDown())
		{
			// TODO: [Shift + 좌클릭] 아이템 나누기 (Split)
			UE_LOG(LogContainer, Warning, TEXT("[%s] 아이템 나누기"), 
				*CachedItemData.ItemDataAsset->DisplayName.ToString());
			return FReply::Handled();
		}
		else if (InMouseEvent.IsAltDown())
		{
			// TODO: [Alt + 좌클릭] 아이템 버리기 (Drop)
			UE_LOG(LogContainer, Warning, TEXT("[%s] 아이템 버리기"), 
				*CachedItemData.ItemDataAsset->DisplayName.ToString());
			return FReply::Handled();
		}
		else
		{
			// TODO: [일반 좌클릭] '드래그 앤 드롭(Drag & Drop)'
			return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
		}
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton) // 우클릭인 경우
	{
		if (!CachedItemData.ItemDataAsset || !OwnerContainer)
		{
			return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
		}

		if (InMouseEvent.IsLeftControlDown())
		{
			// TODO: [Ctrl + 우클릭] 아이템 사용 (Use Item)
			UE_LOG(LogContainer, Warning, TEXT("[%s] 아이템 사용"),
				*CachedItemData.ItemDataAsset->DisplayName.ToString());
			return FReply::Handled();
		}
		else if (InMouseEvent.IsShiftDown())
		{
			// TODO: [Shift + 우클릭] 아이템 세부 정보 표시 (Show Item Detail)
			UE_LOG(LogContainer, Warning, TEXT("[%s] 아이템 세부 정보 표시"),
				*CachedItemData.ItemDataAsset->DisplayName.ToString());
			return FReply::Handled();
		}
		else if (InMouseEvent.IsAltDown())
		{
			// TODO: [Alt + 우클릭] 장비 아이템 장착 (Equip Item)
			UE_LOG(LogContainer, Warning, TEXT("[%s] 장비 아이템 장착"),
				*CachedItemData.ItemDataAsset->DisplayName.ToString());
			return FReply::Handled();
		}
		else
		{
			// TODO: [일반 우클릭] [LCtrl, Shift, Alt, 일반] + [좌, 우 클릭] 으로 가능한 6가지 UI 통합 UI 처리
			UE_LOG(LogContainer, Warning, TEXT("[%s] 아이템 통합 UI"),
				*CachedItemData.ItemDataAsset->DisplayName.ToString());
			return FReply::Handled();
		}
	}

	// 구현하고자 하는 기능 외의 경우, 마찬가지로 무시하고 부모 함수로 처리
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
