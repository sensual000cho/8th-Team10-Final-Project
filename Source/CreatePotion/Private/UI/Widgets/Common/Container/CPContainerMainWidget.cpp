// CPContainerMainWidget.cpp

#include "UI/Widgets/Common/Container/CPContainerMainWidget.h"
#include "Components/Border.h"		// 위젯 Border 헤더
#include "Components/Button.h"		// 위젯 Button 헤더
#include "Blueprint/WidgetLayoutLibrary.h" // DPI 스케일 계산용 헤더

#include "CreatePotion.h"   // 로그용 헤더
#include "Character/CPPlayerController.h"
#include "Components/CPItemContainerComponent.h"
#include "UI/Widgets/Common/Container/CPContainerGridWidget.h"
#include "UI/Widgets/Common/Container/CPMoneyWidget.h"


void UCPContainerMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UCPContainerMainWidget::OnCloseButtonClicked);
	}
}

void UCPContainerMainWidget::BindContainer(UCPItemContainerComponent* InContainer)
{
	if (!InContainer || !ContainerGrid)
	{
		return;
	}

	TargetContainer = InContainer;

	// 연결된 컴포넌트의 Grid(Column * Row) 또는 Slot(개수)에 맞춰 Grid 세팅
	ContainerGrid->InitializeGrid(TargetContainer);

	// 아이템 Container내부 배열이 변경되면 델리게이트 Broadcast 예정, 수신시 UI 업데이트
	TargetContainer->OnContainerUpdated.AddDynamic(this, &UCPContainerMainWidget::UpdateUI);

	if (MoneyWidget)
	{
		if (ACPPlayerController* PC = Cast<ACPPlayerController>(GetOwningPlayer()))
		{
			MoneyWidget->BindInventory(PC->CachedInventoryComponent);
		}
	}

	// 세팅이 끝났으면 UI 업데이트
	UpdateUI();
}

void UCPContainerMainWidget::UpdateUI()
{
	if (TargetContainer && ContainerGrid)
	{
		ContainerGrid->UpdateGrid(TargetContainer->ContainerItems);
	}
}

FReply UCPContainerMainWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 좌클릭을 했을 때
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 마우스가 클릭된 위치가 'DragBorder' 영역 내부인지 검사
		if (DragBorder && DragBorder->GetCachedGeometry().IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
		{
			bIsDraggingWindow = true;

			// 캡처를 하여 마우스가 UI 밖으로 나가도 드래그가 풀리지 않도록 설정
			return FReply::Handled().CaptureMouse(TakeWidget());
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UCPContainerMainWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 드래그 중이고, 이 위젯이 마우스를 캡처하고 있다면
	if (bIsDraggingWindow && this->HasMouseCapture())
	{
		
		float DPIScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());

		// 마우스가 움직인 거리를 DPI로 나눠 해상도가 다르더라도 정확한 이동 수치 계산
		FVector2D ScaledDelta = InMouseEvent.GetCursorDelta() / DPIScale;

		FVector2D NewTranslation = GetRenderTransform().Translation + ScaledDelta;

		SetRenderTranslation(NewTranslation);

		return FReply::Handled();
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UCPContainerMainWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 좌클릭을 뗐을 때
	if (bIsDraggingWindow && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDraggingWindow = false;

		// 마우스 캡처를 Release
		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UCPContainerMainWidget::OnCloseButtonClicked()
{
	HandleCloseRequested();
}

void UCPContainerMainWidget::HandleCloseRequested()
{
	if (ACPPlayerController* PC = Cast<ACPPlayerController>(GetOwningPlayer()))
	{
		PC->CloseExternalContainerUI();
	}
}
