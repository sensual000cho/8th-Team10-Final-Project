// CPContainerMainWidget.cpp

#include "UI/Widgets/Common/Container/CPContainerMainWidget.h"
#include "Components/Border.h"		// 위젯 Border 헤더
#include "Blueprint/WidgetLayoutLibrary.h" // DPI 스케일 계산용 헤더

#include "CreatePotion.h"   // 로그용 헤더
#include "UI/Widgets/Common/Container/CPContainerGridWidget.h"
#include "Components/CPItemContainerComponent.h"

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
		// 마우스가 클릭된 위치가 'DragBorder' 영역 내부인지 검사합니다.
		if (DragBorder && DragBorder->GetCachedGeometry().IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
		{
			bIsDraggingWindow = true;

			// 💡 중요: 캡처(CaptureMouse)를 하면 마우스를 빠르게 휙 움직여서 UI 밖으로 나가도 드래그가 풀리지 않습니다!
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
		// 1. 화면 해상도(DPI) 스케일을 가져옵니다. (4K 화면 등에서도 동일한 이동 속도를 보장하기 위해)
		float DPIScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());

		// 2. 마우스가 움직인 거리(Delta)를 DPI로 나눕니다.
		FVector2D ScaledDelta = InMouseEvent.GetCursorDelta() / DPIScale;

		// 3. 위젯의 현재 RenderTranslation 값에 더해서 실제 화면에서 이동시킵니다.
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

		// 마우스 캡처를 풀어주어 다른 버튼들을 클릭할 수 있게 되돌립니다.
		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}
